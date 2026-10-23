/*
 * v3dshader.c
 *
 * BMA800XL - A bare metal Atari 800XL emulator for Raspberry Pi
 * Copyright (C) 2026 aminch <https://github.com/aminch>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Parts adapted from Mesa (25.2.8), which remain under their
 * license:
 *
 *     src/broadcom/common/v3d_device_info.c:
 *         Copyright © 2016 Broadcom
 *     src/broadcom/vulkan/v3dv_pipeline.c:
 *         Copyright © 2019 Raspberry Pi Ltd
 *     src/gallium/drivers/v3d/v3d_program.c:
 *         Copyright © 2014-2017 Broadcom
 *     src/vulkan/runtime/vk_nir.c:
 *         Copyright © 2015 Intel Corporation
 *         Copyright © 2022 Collabora, LTD
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Modified for BMC64 from BMA800XL's tools/v3dshader/v3dshader.c: "BMA800XL"
 * changed to "BMC64" in two comments.
 *
 * Compiles a vertex and fragment shader pair, as SPIR-V from
 * glslangValidator, into QPU code for V3D 4.2 (the Pi 4) or, with -71,
 * V3D 7.1 (the Pi 5) with Mesa's Broadcom compiler, and writes it out as a
 * C++ header for src/v3d/ (see shader.h there):
 * the fragment shader, the vertex shader, the vertex shader's binning
 * (coordinate) version, their uniforms and the VPM set-up. Runs on a PC,
 * against a built Mesa source tree (see README).
 *
 *   v3dshader NAME VERTEX.spv FRAGMENT.spv > shader_NAME.h
 *   v3dshader -71 NAME VERTEX.spv FRAGMENT.spv > shader71_NAME.h
 *
 * The NIR preparation follows Mesa's Vulkan driver and runtime,
 * src/broadcom/vulkan/v3dv_pipeline.c (preprocess_nir, link_shaders,
 * lower_fs_io, lower_vs_io, the push constant lowering, the NIR options)
 * and src/vulkan/runtime/vk_nir.c; the shader keys follow its OpenGL
 * driver, src/gallium/drivers/v3d/v3d_program.c; the device facts follow
 * src/broadcom/common/v3d_device_info.c. Copyright (C) 2014-2019
 * Broadcom and Raspberry Pi Ltd; vk_nir.c Copyright (C) 2015 Intel
 * Corporation and (C) 2022 Collabora; all MIT licence.
 */

#include "compiler/nir/nir.h"
#include "compiler/nir/nir_builder.h"
#include "compiler/spirv/nir_spirv.h"
#include "broadcom/compiler/v3d_compiler.h"
#include "broadcom/common/v3d_device_info.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const nir_shader_compiler_options nir_options = {
   .lower_uadd_sat = true,
   .lower_usub_sat = true,
   .lower_iadd_sat = true,
   .lower_extract_byte = true,
   .lower_extract_word = true,
   .lower_insert_byte = true,
   .lower_insert_word = true,
   .lower_bitfield_insert = true,
   .lower_bitfield_extract = true,
   .lower_bitfield_reverse = true,
   .lower_bit_count = true,
   .lower_ffract = true,
   .lower_fmod = true,
   .lower_pack_unorm_2x16 = true,
   .lower_pack_snorm_2x16 = true,
   .lower_unpack_unorm_2x16 = true,
   .lower_unpack_snorm_2x16 = true,
   .lower_pack_unorm_4x8 = true,
   .lower_pack_snorm_4x8 = true,
   .lower_unpack_unorm_4x8 = true,
   .lower_unpack_snorm_4x8 = true,
   .lower_pack_half_2x16 = true,
   .lower_unpack_half_2x16 = true,
   .lower_pack_32_2x16 = true,
   .lower_pack_32_2x16_split = true,
   .lower_unpack_32_2x16_split = true,
   .lower_mul_2x32_64 = true,
   .lower_fdiv = true,
   .lower_find_lsb = true,
   .lower_ffma16 = true,
   .lower_ffma32 = true,
   .lower_ffma64 = true,
   .lower_flrp32 = true,
   .lower_fpow = true,
   .lower_fsqrt = true,
   .lower_ifind_msb = true,
   .lower_isign = true,
   .lower_ldexp = true,
   .lower_mul_high = true,
   .lower_wpos_pntc = false,
   .lower_to_scalar = true,
   .lower_device_index_to_zero = true,
   .lower_fquantize2f16 = true,
   .lower_ufind_msb = true,
   .lower_fsat = true,
   .has_fsub = true,
   .has_isub = true,
   .has_uclz = true,
   .vertex_id_zero_based = false,
   .lower_interpolate_at = true,
   .max_unroll_iterations = 16,
   .force_indirect_unrolling = (nir_var_shader_in | nir_var_function_temp),
   .discard_is_demote = true,
   .scalarize_ddx = true,
};

static const struct spirv_to_nir_options spirv_options = {
   .ubo_addr_format = nir_address_format_32bit_index_offset,
   .ssbo_addr_format = nir_address_format_32bit_index_offset,
   .phys_ssbo_addr_format = nir_address_format_2x32bit_global,
   .push_const_addr_format = nir_address_format_logical,
   .shared_addr_format = nir_address_format_32bit_offset,
};

static uint32_t *
read_file(const char *path, size_t *words)
{
   FILE *f = fopen(path, "rb");
   if (!f) {
      fprintf(stderr, "Cannot open %s\n", path);
      exit(1);
   }
   fseek(f, 0, SEEK_END);
   long size = ftell(f);
   fseek(f, 0, SEEK_SET);
   uint32_t *data = malloc(size);
   if (fread(data, 1, size, f) != (size_t)size) {
      fprintf(stderr, "Cannot read %s\n", path);
      exit(1);
   }
   fclose(f);
   *words = size / 4;
   return data;
}

static int
type_size_vec4(const struct glsl_type *type, bool bindless)
{
   return glsl_count_attribute_slots(type, false);
}

/* Push constants are the shader's uniforms, at byte offsets. */
static bool
lower_push_constants(nir_builder *b, nir_intrinsic_instr *intr, void *data)
{
   if (intr->intrinsic != nir_intrinsic_load_push_constant)
      return false;
   intr->intrinsic = nir_intrinsic_load_uniform;
   return true;
}


/* A texture's index is its binding; it is sampled with its own sampler. */
static bool
lower_textures(nir_builder *b, nir_tex_instr *tex, void *data)
{
   bool progress = false;
   for (int i = tex->num_srcs - 1; i >= 0; i--) {
      if (tex->src[i].src_type != nir_tex_src_texture_deref &&
          tex->src[i].src_type != nir_tex_src_sampler_deref)
         continue;
      nir_deref_instr *deref = nir_src_as_deref(tex->src[i].src);
      nir_variable *var = nir_deref_instr_get_variable(deref);
      if (tex->src[i].src_type == nir_tex_src_texture_deref)
         tex->texture_index = var->data.binding;
      else
         tex->sampler_index = var->data.binding;
      nir_tex_instr_remove_src(tex, i);
      progress = true;
   }
   return progress;
}

/* As v3dv's preprocess_nir and pipeline_lower_nir. */
static void
preprocess(nir_shader *nir)
{
   const struct nir_lower_sysvals_to_varyings_options sysvals = {
      .frag_coord = true,
      .point_coord = true,
   };
   NIR_PASS(_, nir, nir_lower_sysvals_to_varyings, &sysvals);
   nir->info.separate_shader = true;
   NIR_PASS(_, nir, nir_lower_variable_initializers, nir_var_shader_out);
   if (nir->info.stage == MESA_SHADER_FRAGMENT)
      NIR_PASS(_, nir, nir_opt_vectorize_io_vars, nir_var_shader_out);
   NIR_PASS(_, nir, nir_lower_io_vars_to_temporaries,
            nir_shader_get_entrypoint(nir), true, false);
   NIR_PASS(_, nir, nir_lower_system_values);
   NIR_PASS(_, nir, nir_lower_alu_to_scalar, NULL, NULL);
   NIR_PASS(_, nir, nir_lower_global_vars_to_local);
   NIR_PASS(_, nir, nir_split_var_copies);
   NIR_PASS(_, nir, nir_split_struct_vars, nir_var_function_temp);
   v3d_optimize_nir(NULL, nir);
   NIR_PASS(_, nir, nir_lower_explicit_io, nir_var_mem_push_const,
            nir_address_format_32bit_offset);
   NIR_PASS(_, nir, nir_lower_load_const_to_scalar);
   NIR_PASS(_, nir, nir_lower_var_copies);
   NIR_PASS(_, nir, nir_lower_indirect_derefs, nir_var_shader_in, UINT32_MAX);
   NIR_PASS(_, nir, nir_lower_indirect_derefs, nir_var_function_temp, 2);
   NIR_PASS(_, nir, nir_lower_frexp);
   v3d_optimize_nir(NULL, nir);
}

static void
lower_resources(nir_shader *nir)
{
   nir_shader_gather_info(nir, nir_shader_get_entrypoint(nir));
   NIR_PASS(_, nir, nir_shader_intrinsics_pass, lower_push_constants,
            nir_metadata_control_flow, NULL);
   NIR_PASS(_, nir, nir_shader_tex_pass, lower_textures,
            nir_metadata_control_flow, NULL);
}

/* As v3dv's link_shaders. */
static void
link_shaders(nir_shader *producer, nir_shader *consumer)
{
   NIR_PASS(_, producer, nir_lower_io_vars_to_scalar, nir_var_shader_out);
   NIR_PASS(_, consumer, nir_lower_io_vars_to_scalar, nir_var_shader_in);
   nir_lower_io_array_vars_to_elements(producer, consumer);
   v3d_optimize_nir(NULL, producer);
   v3d_optimize_nir(NULL, consumer);
   if (nir_link_opt_varyings(producer, consumer))
      v3d_optimize_nir(NULL, consumer);
   NIR_PASS(_, producer, nir_remove_dead_variables, nir_var_shader_out, NULL);
   NIR_PASS(_, consumer, nir_remove_dead_variables, nir_var_shader_in, NULL);
   if (nir_remove_unused_varyings(producer, consumer)) {
      NIR_PASS(_, producer, nir_lower_global_vars_to_local);
      NIR_PASS(_, consumer, nir_lower_global_vars_to_local);
      v3d_optimize_nir(NULL, producer);
      v3d_optimize_nir(NULL, consumer);
      NIR_PASS(_, producer, nir_remove_dead_variables, nir_var_shader_out, NULL);
      NIR_PASS(_, consumer, nir_remove_dead_variables, nir_var_shader_in, NULL);
   }
}

/* As v3dv's lower_fs_io and lower_vs_io. */
static void
lower_fs_io(nir_shader *nir)
{
   NIR_PASS(_, nir, nir_lower_io_array_vars_to_elements_no_indirects, false);
   NIR_PASS(_, nir, nir_remove_dead_variables, nir_var_shader_out, NULL);
   nir_assign_io_var_locations(nir, nir_var_shader_in, &nir->num_inputs,
                               MESA_SHADER_FRAGMENT);
   nir_assign_io_var_locations(nir, nir_var_shader_out, &nir->num_outputs,
                               MESA_SHADER_FRAGMENT);
   NIR_PASS(_, nir, nir_lower_io, nir_var_shader_in | nir_var_shader_out,
            type_size_vec4, 0);
}

static void
lower_vs_io(nir_shader *nir)
{
   NIR_PASS(_, nir, nir_lower_io_array_vars_to_elements_no_indirects, false);
   nir_assign_io_var_locations(nir, nir_var_shader_in, &nir->num_inputs,
                               MESA_SHADER_VERTEX);
   nir_assign_io_var_locations(nir, nir_var_shader_out, &nir->num_outputs,
                               MESA_SHADER_VERTEX);
}

static nir_shader *
load(const char *path, gl_shader_stage stage)
{
   size_t words;
   uint32_t *spirv = read_file(path, &words);
   nir_shader *nir = spirv_to_nir(spirv, words, NULL, 0, stage, "main",
                                  &spirv_options, &nir_options);
   if (!nir) {
      fprintf(stderr, "%s: not valid SPIR-V\n", path);
      exit(1);
   }
   /* As Mesa's Vulkan runtime, src/vulkan/runtime/vk_nir.c. */
   NIR_PASS(_, nir, nir_lower_variable_initializers, nir_var_function_temp);
   NIR_PASS(_, nir, nir_lower_returns);
   NIR_PASS(_, nir, nir_inline_functions);
   NIR_PASS(_, nir, nir_copy_prop);
   NIR_PASS(_, nir, nir_opt_deref);
   nir_remove_non_entrypoints(nir);
   NIR_PASS(_, nir, nir_lower_variable_initializers, ~0);
   NIR_PASS(_, nir, nir_split_var_copies);
   NIR_PASS(_, nir, nir_split_per_member_structs);
   NIR_PASS(_, nir, nir_remove_dead_variables,
            nir_var_shader_in | nir_var_shader_out | nir_var_system_value,
            NULL);
   NIR_PASS(_, nir, nir_lower_clip_cull_distance_array_vars);
   preprocess(nir);
   return nir;
}

static void
debug_output(const char *msg, void *data)
{
   fprintf(stderr, "%s\n", msg);
}

/* The uniform kinds the BMC64 driver fills in (src/v3d/shader.h). */
static const char *
uniform_kind(enum quniform_contents c)
{
   switch (c) {
   case QUNIFORM_CONSTANT: return "UniformConstant";
   case QUNIFORM_UNIFORM: return "UniformParameter";
   case QUNIFORM_VIEWPORT_X_SCALE: return "UniformViewportXScale";
   case QUNIFORM_VIEWPORT_Y_SCALE: return "UniformViewportYScale";
   case QUNIFORM_VIEWPORT_Z_OFFSET: return "UniformViewportZOffset";
   case QUNIFORM_VIEWPORT_Z_SCALE: return "UniformViewportZScale";
   case QUNIFORM_TMU_CONFIG_P0: return "UniformTmuConfigP0";
   case QUNIFORM_TMU_CONFIG_P1: return "UniformTmuConfigP1";
   case QUNIFORM_TEXTURE_WIDTH: return "UniformTextureWidth";
   case QUNIFORM_TEXTURE_HEIGHT: return "UniformTextureHeight";
   default:
      fprintf(stderr, "Uniform kind %d isn't supported\n", c);
      exit(1);
   }
}

static struct v3d_prog_data *
compile(const struct v3d_compiler *compiler, struct v3d_key *key,
        nir_shader *nir, const char *name, const char *what)
{
   struct v3d_prog_data *prog = NULL;
   uint32_t size = 0;
   uint64_t *code = v3d_compile(compiler, key, &prog,
                                nir_shader_clone(NULL, nir), debug_output,
                                NULL, 0, 0, &size);
   if (!code) {
      fprintf(stderr, "%s: the %s shader didn't compile\n", name, what);
      exit(1);
   }

   printf("static const uint64_t %s_%s_code[] = {\n", name, what);
   for (uint32_t i = 0; i < size / 8; i++)
      printf("    0x%016llxull,\n", (unsigned long long)code[i]);
   printf("};\n\n");
   printf("static const v3d::Uniform %s_%s_uniforms[] = {\n", name, what);
   for (uint32_t i = 0; i < prog->uniforms.count; i++)
      printf("    {v3d::%s, 0x%08x},\n",
             uniform_kind(prog->uniforms.contents[i]), prog->uniforms.data[i]);
   if (prog->uniforms.count == 0)
      printf("    {v3d::UniformConstant, 0},\n");
   printf("};\n\n");
   return prog;
}

static void
write_shader(const char *name, const char *what, struct v3d_prog_data *prog,
             bool vertex)
{
   if (prog->spill_size) {
      fprintf(stderr, "%s: the %s shader spills registers\n", name, what);
      exit(1);
   }
   printf("    {%s_%s_code, sizeof %s_%s_code / 8, %s_%s_uniforms, %u, %u, "
          "%s,\n", name, what, name, what, name, what, prog->uniforms.count,
          prog->threads, prog->single_seg ? "true" : "false");
   if (vertex) {
      struct v3d_vs_prog_data *vs = (struct v3d_vs_prog_data *)prog;
      for (unsigned i = 1; i < ARRAY_SIZE(vs->vattr_sizes); i++) {
         if (vs->vattr_sizes[i]) {
            fprintf(stderr, "%s: only one vertex attribute is supported\n",
                    name);
            exit(1);
         }
      }
      printf("     %u, %u, %s, %s, %s, %u},\n", vs->vpm_input_size,
             vs->vpm_output_size, vs->separate_segments ? "true" : "false",
             vs->uses_vid ? "true" : "false", vs->uses_iid ? "true" : "false",
             vs->vattr_sizes[0]);
   } else {
      printf("     0, 0, false, false, false, 0},\n");
   }
}

int
main(int argc, char **argv)
{
   /* V3D 4.2 on the BCM2711; V3D 7.1 on the BCM2712, with revision 0 so
    * that no instruction added in a later revision is used, and the VPM
    * size the BMC64 driver checks the GPU has.
    */
   struct v3d_device_info devinfo = {
      .ver = 42, .rev = 0, .vpm_size = 16384, .qpu_count = 8,
      .has_accumulators = true, .clipper_xy_granularity = 256.0f,
      .cle_readahead = 256, .cle_buffer_min_size = 4096,
   };
   if (argc == 5 && strcmp(argv[1], "-71") == 0) {
      devinfo.ver = 71;
      devinfo.has_accumulators = false;
      devinfo.clipper_xy_granularity = 64.0f;
      devinfo.cle_readahead = 1024;
      devinfo.cle_buffer_min_size = 16384;
      argv++;
      argc--;
   }
   if (argc != 4) {
      fprintf(stderr, "usage: v3dshader [-71] NAME VERTEX.spv FRAGMENT.spv\n");
      return 1;
   }
   const char *name = argv[1];
   glsl_type_singleton_init_or_ref();

   const struct v3d_compiler *compiler = v3d_compiler_init(&devinfo, 0);

   nir_shader *vs = load(argv[2], MESA_SHADER_VERTEX);
   nir_shader *fs = load(argv[3], MESA_SHADER_FRAGMENT);
   link_shaders(vs, fs);
   lower_resources(fs);
   lower_fs_io(fs);
   lower_resources(vs);
   lower_vs_io(vs);

   const char *vbase = strrchr(argv[2], '/'), *fbase = strrchr(argv[3], '/');
   printf("// Generated by tools/v3dshader from %s and %s for V3D %d.%d. "
          "Do not edit.\n\n", vbase ? vbase + 1 : argv[2],
          fbase ? fbase + 1 : argv[3], devinfo.ver / 10, devinfo.ver % 10);

   struct v3d_fs_key fkey;
   memset(&fkey, 0, sizeof fkey);
   fkey.cbufs = 1;
   fkey.logicop_func = PIPE_LOGICOP_COPY;
   struct v3d_fs_prog_data *fp =
      (struct v3d_fs_prog_data *)compile(compiler, &fkey.base, fs, name, "fs");

   struct v3d_vs_key vkey;
   memset(&vkey, 0, sizeof vkey);
   vkey.base.is_last_geometry_stage = true;
   for (int i = 0; i < fp->num_inputs; i++)
      vkey.used_outputs[vkey.num_used_outputs++] = fp->input_slots[i];
   struct v3d_vs_prog_data *vp =
      (struct v3d_vs_prog_data *)compile(compiler, &vkey.base, vs, name, "vs");

   struct v3d_vs_key ckey;
   memset(&ckey, 0, sizeof ckey);
   ckey.base.is_last_geometry_stage = true;
   ckey.is_coord = true;
   struct v3d_vs_prog_data *cp =
      (struct v3d_vs_prog_data *)compile(compiler, &ckey.base, vs, name, "cs");

   struct vpm_config bin, render;
   if (!v3d_compute_vpm_config(&devinfo, cp, vp, NULL, NULL, &bin, &render)) {
      fprintf(stderr, "%s: no VPM configuration fits\n", name);
      return 1;
   }

   printf("static const v3d::Program %s = {\n", name);
   write_shader(name, "fs", &fp->base, false);
   write_shader(name, "vs", &vp->base, true);
   write_shader(name, "cs", &cp->base, true);
   printf("    // fragment shader: varyings, flat shading flags, writes Z, "
          "no early Z,\n    // centre W, scoreboard wait on first thread "
          "switch, point/line varyings\n");
   printf("    %u, 0x%x, %s, %s, %s, %s, %s,\n", fp->num_inputs,
          fp->flat_shade_flags[0], fp->writes_z ? "true" : "false",
          fp->disable_ez ? "true" : "false",
          fp->uses_center_w ? "true" : "false",
          fp->lock_scoreboard_on_first_thrsw ? "true" : "false",
          fp->uses_implicit_point_line_varyings ? "true" : "false");
   printf("    // VPM: binning As, Ve, Vc; rendering As, Ve, Vc\n");
   printf("    {%u, %u, %u}, {%u, %u, %u},\n", bin.As, bin.Ve, bin.Vc,
          render.As, render.Ve, render.Vc);
   printf("};\n");
   return 0;
}

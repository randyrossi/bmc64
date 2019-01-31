#!/usr/bin/perl -s
#
# analyze-vicerc <vicerc> ...
#
# find common options in different vicerc
#

if(scalar(@ARGV)==0) {
    print STDERR "Usage: $0 <vicerc files> ...\n";
    exit 1;
}

# read vicerc files
my %all = ();
my $bit = 1;
my @emus = ();
my %emu_file = ();
foreach(@ARGV) {
    my $emu = load_vicerc($_,\%all,$bit);
    die if($emu eq "");
    $emu_file{$emu} = $_;
    push @emus,$emu;
    $bit = $bit * 2;
}

# map each file to a bitmask
my %bits = ();
$bit = 1;
foreach(@emus) {
    $bits{$_} = $bit;
    print sprintf("%8s: %3d  %s\n",$_,$bit,$emu_file{$_});
    $bit = $bit * 2;
}
my $max = $bit - 1;
print "max perm: $max\n\n";

# now sort in resources per mask
my %res_per_mask;
foreach(keys(%all)) {
    my $bitmask = $all{$_};
    if(!defined($res_per_mask{$bitmask})) {
        $res_per_mask{$bitmask} = [ $_ ];
    } else {
        my $array = $res_per_mask{$bitmask};
        push @$array, $_;
    }
}

my @perms = sort(keys(%res_per_mask));
foreach(@perms) {
    
    my $mask = $_;
    
    # generate name for permutation
    my @perm_emus = ();
    $bit = 1;
    foreach(@emus) {
        if(($mask & $bit) != 0) {
            push @perm_emus,$_;
        }
        $bit = $bit * 2;
    }
    my $perm_name = "_" . join("_",@perm_emus) . "_";
    if($mask == $max) {
        $perm_name = "Common";
    }
    
    # find resources for permutation
    my $array = $res_per_mask{$_};
    my @sorted_res;
    @sorted_res = sort(@$array);
    
    # output
    print "$perm_name\n";
    foreach(@sorted_res) {
        print "\t$_\n";
    }
    print "\n";
}

# ----- sub routines -----

# load a vicerc into a dictionary
sub load_vicerc
{
    my $filename = shift;
    my $all = shift;
    my $bit = shift;
    
    open(FH,"<$filename") || die "can't open '$filename'";
    my @input;
    @input = <FH>;
    close(FH);
    
    my $name = "";
    pop @input;
    foreach(@input) {
        chomp;
        if(m/(\S+)=(.*)$/) {
            if(!defined($all->{$1})) {
                $all->{$1} = $bit;
            } else {
                $all->{$1} |= $bit;
            }
        } elsif(m/\[(\S+)\]/) {
            $name = $1;
        }
    }
    return $name;
}

# Contributing

Thanks for your interest in contributing! Contributions of all kinds are welcome, including bug fixes, new features, documentation improvements, and other enhancements.

Before starting a significant change, please open an issue or discussion to describe what you would like to change. This helps avoid duplicated work and gives maintainers and contributors an opportunity to discuss the proposed approach. Please also read the [project scope](/docs/SCOPE.md) to check that your change fits the intended direction of BMC64.

Please follow the project's [Code of Conduct](/docs/CODE_OF_CONDUCT.md) in all interactions with the community.

## Pull Request Process

1. **Fork and branch**
   Fork the repository and create a new branch for your changes. Use a clear, descriptive branch name.

2. **Make your changes**
   Keep changes focused on a single issue or purpose where possible. Follow the existing code style and project conventions.

3. **Review your own changes**
   Before submitting a pull request, review your changes carefully. Make sure you understand the code you are submitting and check for unnecessary changes, errors, security issues, and unintended behaviour.

   If you used AI-assisted development tools to generate, modify, or suggest code, you are still responsible for the resulting contribution. Review and understand the generated code, verify that it is correct and appropriate for the project, and test it before submitting it. Do not submit AI-generated code that you have not personally reviewed.

4. **Test your changes**
   Test your changes before submitting a pull request and verify that they do not introduce regressions. Run any existing tests and add or update tests where appropriate.

   In your pull request, clearly state which Raspberry Pi models you have tested your changes on. Supported hardware ranges from the **Raspberry Pi Zero through to the Raspberry Pi 3 Model B+**.

   You are not expected to test on every Raspberry Pi model, but please list the models you have personally tested. If your changes have not been tested on physical hardware, state this clearly in the pull request.

5. **Update documentation**
   Update the README or other relevant documentation when your changes affect installation, configuration, environment variables, commands, APIs, user-facing behaviour, hardware requirements, or other documented functionality.

6. **Commit clearly**
   Use concise, descriptive commit messages that explain the purpose of your changes.

7. **Open a Pull Request against `master`**
   All pull requests must target the `master` branch. Provide a clear description of what changed and why, and link any related issues where applicable.

   Your pull request should also include the Raspberry Pi model(s) on which the changes were tested.

8. **Address reviews and checks**
   Ensure automated checks pass and respond to review feedback. Additional changes may be requested before the pull request is approved.

9. **Merge**
   Once the pull request has been approved and all required checks have passed, a project maintainer will merge it.

## Reporting Bugs

Before opening a bug report, please check [existing issues](https://github.com/randyrossi/bmc64/issues) to see whether the problem has already been reported.

Because BMC64 is built on VICE 3.3, many emulation bugs originate in VICE itself. Before reporting, please try to narrow down where the problem comes from:

- **VICE 3.3**: Check whether the problem also happens in VICE 3.3 on a PC.
- **Latest VICE**: Check whether it still happens in the [latest VICE release](https://vice-emu.sourceforge.io/). If it was fixed there, mention which version.
- **VICE bug tracker**: [Search for](https://sourceforge.net/p/vice-emu/bugs/) an existing report or discussion of the same problem, and link it in your issue.
- **BMC64 v4.2**: Check whether the problem also happens in BMC64 v4.2. If it works there but not in the current version, it is a regression. Please say so and note the last version you know it worked in.

You are not expected to be able to test all of these, but the more information you include the easier it is to resolve issues.

When reporting a bug, please use the [bug report issue template](https://github.com/randyrossi/bmc64/issues/new?template=bug_report.yml) and be as detailed as you can about how to reproduce the problem: the machine and settings used, the exact software, how it was loaded, and every step taken in order. Issues that can't be reproduced are very hard to fix.

## Suggesting Features

Feature requests are welcome. Before suggesting a feature, please read the [project scope](/docs/SCOPE.md), which describes BMC64's priorities and what is out of scope. Requests that don't align with the current scope may be declined or closed.

Please use the [feature request issue template](https://github.com/randyrossi/bmc64/issues/new?template=feature_request.yml) to describe the proposed feature, the problem it would solve, and any relevant examples or alternatives.

Thank you for helping improve the project!

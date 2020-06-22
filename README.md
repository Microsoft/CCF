# The Confidential Consortium Framework 

[![Gitter](https://badges.gitter.im/MSRC-CCF/community.svg)](https://gitter.im/MSRC-CCF/community?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge) [![Build Status](https://dev.azure.com/MSRC-CCF/CCF/_apis/build/status/CCF%20Github%20CI?branchName=master)](https://dev.azure.com/MSRC-CCF/CCF/_build/latest?definitionId=3&branchName=master)
[![Docs](https://img.shields.io/badge/Docs-succeeded-green)](https://microsoft.github.io/CCF)


<img alt="ccf" align="right" src="https://microsoft.github.io/CCF/master/_images/ccf.svg" width="300">

The Confidential Consortium Framework (CCF) is an open-source framework for building a new category of secure, highly available,
and performant applications that focus on multi-party compute and data.
CCF can enable high-scale, confidential networks that meet key enterprise requirements
— providing a means to accelerate production and enterprise adoption of consortium based blockchain and multi-party compute technology.

Leveraging the power of trusted execution environments (TEEs), decentralized systems concepts, and cryptography, CCF enables enterprise-ready multiparty computation or blockchains.

## Learn more and get started

 * Browse the [CCF Documentation](https://microsoft.github.io/CCF/).
 * Read the [CCF Technical Report](CCF-TECHNICAL-REPORT.pdf) for a more detailed description.
 * Learn more about [Azure Confidential Computing](https://azure.microsoft.com/solutions/confidential-compute/) offerings like Azure DC-series (which support Intel SGX TEE)
   and [Open Enclave](https://github.com/openenclave/openenclave) that CCF can run on.

## Getting Started with CCF

* [Get started](https://microsoft.github.io/CCF/master/quickstart/index.html) with Azure confidentual computing and CCF.
* Learn how to [build](https://microsoft.github.io/CCF/master/quickstart/build.html) and [run](https://microsoft.github.io/CCF/master/quickstart/build.html#running-tests) a test network.
* Start [writing](https://microsoft.github.io/CCF/master/developers/index.html) your own CCF application.
* Submit [bugs](https://github.com/microsoft/CCF/issues/new?assignees=&labels=bug&template=bug_report.md&title=) and [feature requests](https://github.com/microsoft/CCF/issues/new?assignees=&labels=enhancement&template=feature_request.md&title=), and help us verify those that are checked in.

## Third-party components

We rely on several open source third-party components, attributed under [THIRD_PARTY_NOTICES](THIRD_PARTY_NOTICES.txt).

## Contributing

This project welcomes contributions and suggestions. Most contributions require you to
agree to a Contributor License Agreement (CLA) declaring that you have the right to,
and actually do, grant us the rights to use your contribution. For details, visit
https://cla.microsoft.com.

When you submit a pull request, a CLA-bot will automatically determine whether you need
to provide a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the
instructions provided by the bot. You will only need to do this once across all repositories using our CLA.

All pull requests must pass a suite of CI tests before they will be merged.
The test commands are defined in [`test.yml`](https://github.com/microsoft/CCF/blob/master/.azure-pipelines-templates/test.yml), so you can locally repeat any tests which fail.
You should at least run the code format checking scripts defined in
[`checks.yml`](https://github.com/microsoft/CCF/blob/master/.azure-pipelines-templates/checks.yml) before creating a pull request, ensuring all of your code is correctly formatted.
The test commands will only report misformatted files - to _reformat_ the files, pass `-f` to the `check-format.sh ...` command and remove `--check` from the `black ...` command.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/)
or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.


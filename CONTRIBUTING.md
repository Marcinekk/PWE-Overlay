# Contributing to PWE Overlay

Thank you for your interest in contributing!

## Reporting Issues
- Open an issue on GitHub using the [bug report template](.github/ISSUE_TEMPLATE/bug_report.md).
- Include:
    - Game version (ATS / ETS2)
    - Plugin version
    - OS version
    - Steps to reproduce
    - Screenshots if applicable

## Feature Requests
- Open an issue using the [feature request template](.github/ISSUE_TEMPLATE/feature_request.md).
- Explain the requested feature clearly and provide context.

## Pull Requests
- Please open an issue first before submitting a pull request.
- Fork the repository and create a feature branch:
    ```bash
        git checkout -b feature/your-feature
    ```
- Ensure your branch builds successfully.
- Make sure you do not commit build outputs (/build or /dist folders).
- Follow the existing code style:
    - TypeScript / Vue conventions for frontend
    - C++ conventions for backend

## Code Guidelines
- Use meaningful commit messages.
- Keep PRs focused and concise.
- Run tests (if applicable) before submitting.

## Third-Party Dependencies
- Respect the licenses of third-party libraries.
- Include new third-party libraries in [THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md).
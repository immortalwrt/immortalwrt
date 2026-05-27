FROM mcr.microsoft.com/devcontainers/base:noble

USER root

RUN apt-get update \
    && apt-get install -y --no-install-recommends ca-certificates curl \
    && rm -rf /var/lib/apt/lists/*

RUN curl -fsSL https://build-scripts.immortalwrt.org/init_build_environment.sh -o /tmp/init_build_environment.sh \
    && bash /tmp/init_build_environment.sh \
    && rm -f /tmp/init_build_environment.sh

USER vscode
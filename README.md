# Graphs

A simple utility for graphs analysis.
[![CI](https://github.com/victorbaldin56/Graphs/actions/workflows/main.yml/badge.svg)](https://github.com/victorbaldin56/Graphs/actions/workflows/main.yml)

## Overview

A graph builder and dominators and postdominators detector.

## Features

![alt text](img/graph.png)
![alt text](img/dom.png)
![alt text](img/pdom.png)

## Building

It is highly recommended to use pre-built Docker image.

### With Docker

1. Pull the pre-built Docker image and start the Docker container using it. Recommended command:

   ```sh
   docker run --restart unless-stopped -it -v [YOUR_BACKUP_VOLUME_PATH]:/home/dev --name linux_dev victorbaldin56/linux_dev:a51a343886c34b73ddf5344e72ce095c416af046
   ```

1. Clone the repo.

   ```sh
   git clone https://github.com/victorbaldin56/Graphs.git
   ```

1. Install additional project dependencies with Conan

   ```sh
   python3 -m venv .venv
   source .venv/bin/activate
   pip install conan==2.23.0
   conan profile detect --force
   conan install . --build=missing --output-folder=build -pr:a=conan_profiles/release
   ```

1. Configure the build

   ```sh
   cmake --preset conan-release
   ```

1. Build

   ```sh
   cmake --build build -j
   ```

### Have no Docker yet?

> [!TIP]
> It will be probably easier for you to install Docker because some tool
> requirements are specific.

You can check my [Dockerfile](https://github.com/victorbaldin56/DockerLinuxDev/blob/a51a343886c34b73ddf5344e72ce095c416af046/Dockerfile)
and build the same environment yourself.

## Usage

To get quick help on available commands, run:

```sh
./build/graphs_main --help
```

## License

This project is licensed after APACHE 2.0 license.

## Contribution

This project is currently in active development. If you wish to contribute
you are welcome to submit pull requests and issues. Thank you!

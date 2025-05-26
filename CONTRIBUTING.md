# Contributing

Thank you for your interest in contributing to this project!

We welcome contributions to improve the code, fix bugs, add features, or enhance documentation.

## Guidelines

### 🛠 Code Style

- Use **snake_case** for all **class names**, **function names**, and **parameters**.
- **Always format your code** using the provided `.clang-format` file **before pushing**.

### 📁 File Organization

- Place header files in the `include/` directory.
- Place source (`.cpp`) files in the `src/` directory.
- Group related code into logical modules or folders where appropriate.

- C source is located in `include/ecos/ecos.h` and `src/ecos/ecos.cpp`
- Python code is located in `ecospy` folder.

- Examples are located in `/examples`.

## How to Contribute

1. **Fork** the repository and clone your fork:

    ```bash
    git clone https://github.com/your-username/your-fork.git
    cd your-fork
    ```

2. **Create a new branch** for your change:

    ```bash
    git checkout -b feature/your_feature_name
    ```

3. **Make your changes**, following the code style guidelines.

4. **Test your changes** to ensure they do not break existing functionality.

5. **Commit** your changes with a clear and concise commit message.

    ```bash
    git commit -m "Add feature: short description"
    ```

6. **Push** your branch to your fork:

    ```bash
    git push origin feature/your_feature_name
    ```

7. **Open a pull request** to the `master` branch of the upstream repository.  
   Include a clear description of your changes and reference any relevant issues.


## Licensing

Unless explicitly stated otherwise, any contribution you submit will be licensed under the same open-source license that governs this repository (see `LICENSE`). By submitting a pull request, you agree that you have the right to license your code under these terms.

## Need Help?

If you're unsure how to get started or have questions, feel free to [open an issue](https://github.com/your-org/your-repo/issues).

We appreciate your contributions!

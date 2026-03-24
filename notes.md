# X-NCU Codebase Notes for Beginners

## Introduction

The **X-NCU** application is a Twitter-like social media platform tailored for a university setting. The backend is written in modern **C++** using the **Crow** microframework, which behaves similarly to Express.js in Node.js or Flask in Python. 

This project is an excellent sandbox for learning how to build web servers, handle HTTP REST APIs, and manage simple file-based data persistency in C++.

## Project Structure

Here is an overview of the key files and directories:

*   **`main.cpp`**: The core application server. It contains the data models (like `user` and `post`) and sets up all the Web API routes (endpoints).
*   **`CMakeLists.txt`**: The build system configuration file. It instructs the CMake compiler on how to build the executable, link necessary libraries like OpenSSL and Boost, and copy frontend assets to the build destination.
*   **`otp_service.h`**: A standalone service for generating and emailing One Time Passwords (OTPs) via socket programming and TLS.
*   **`sha256.h`**: A cryptographic utility for hashing passwords securely before storing them.
*   **`database/`**: A directory containing `.csv` files acting as a simple, custom-built database (e.g., `users.csv`, `posts.csv`).
*   **`templates/` & `static/`**: Contains the frontend files: HTML layouts (rendered via Mustache), CSS for styling, and Vanilla JavaScript.

## Core C++ Concepts to Learn from This Codebase

### 1. The Crow Framework Handle Routing

Crow routes act as the bridge between the frontend and your C++ backend. In `main.cpp`, you will see routes defined utilizing C++ lambdas (anonymous functions):
```cpp
CROW_ROUTE(app, "/login").methods(crow::HTTPMethod::POST)
([](const crow::request& req) {
    // Process the login request and return a response
});
```
This tells the C++ server to listen for HTTP POST requests specifically at the `/login` URL pathway.

### 2. Classes and Object-Oriented Programming (OOP)

The codebase uses object-oriented models to encapsulate data and behavior. Pay close attention to these constructs:

*   **`user` class**: Represents an account. Its constructor reads from `users.csv` to find and populate its private fields based on a provided ID, demonstrating robust file I/O operations (`std::ifstream`).
*   **`post` class**: Represents a tweet/post. It contains logic to sanitize input (replacing newlines and commas) so the user's text doesn't accidentally break the underlying CSV formatting.

### 3. File-Based Database (CSV Parsing)

Instead of a heavyweight database server like MySQL or PostgreSQL, this project uses plain text `.csv` files. You will observe heavy use of the `<fstream>` and `<sstream>` libraries:

*   `std::ifstream` / `std::ofstream`: Used to open, read, and append to files.
*   `std::stringstream`: Crucial for breaking down a comma-separated line.
*   `getline(ss, token, ',')`: A frequent paradigm used to extract a piece of data up to the next comma.

*Note: This approach is excellent for beginners to understand data structures and persistence, though a real database (via an SQL driver) is standard in industry production.*

### 4. Security Practices (Hashing & OTP)

*   **Hashing**: Notice how passwords are never stored in plain text. A function like `sha256(password)` is applied before writing credentials to the CSV.
*   **OTP Emailing**: `otp_service.h` connects directly to Google's SMTP server via **Boost.Asio**. This introduces networking and Multithreading—notice the use of `std::mutex` to prevent "race conditions" from occurring when multiple users request OTPs concurrently!

### 5. Web Templates

The backend renders dynamic HTML and sends it to the user using Crow's built-in **Mustache** integration. Look out for code like this:
```cpp
crow::mustache::context ctx;
ctx["user_name"] = currentUser.fullname();
return crow::mustache::load("index.html").render(ctx);
```
This cleanly replaces the placeholder `{{user_name}}` in the `index.html` file with the actual C++ string variable before serving the page to the browser.

## Next Steps for Your Journey

1.  **Analyze `main.cpp` Thoroughly**: Review how `authenticateUser()` and `registerUser()` manipulate strings and interact with files. Notice error handling concepts like returning `-1` or `-2` for different failure states.
2.  **Understand Memory Management**: Notice how C++ standard library strings (`std::string`) are frequently passed by constant reference (e.g., `const string& val`) to getter and setter methods. This avoids the performance hit of copying large amounts of text in memory.
3.  **Try Extending It!**: Challenge yourself. Try adding a "Delete Post" feature. You would need to add a new Crow route, read `posts.csv`, filter out the specific line possessing the targeted post ID, and rewrite the remainder of the file.

Happy coding!

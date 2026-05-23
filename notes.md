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

## Additional Core Features (Discovered in Full Analysis)

### 6. Social Features (Likes, Follows, Lists)

The application implements a rich social feature set:

- **Like/Unlike System**: Users can like/unlike posts. Helper functions track liked posts per user:
  - `hasUserLikedPost(userId, postId)` — checks if a user has liked a specific post
  - `addLikeToUser()` — records a like in the user's liked_post_ids field
  - `removeLikeFromUser()` — removes a like from the user's liked_post_ids field
  - Posts stored in CSV have a `liked_post_ids` field: semicolon-separated list of post IDs

- **Follow/Unfollow System**: Users can follow other users. Tracking stored in user CSV:
  - `doesUserFollow(userA, userB)` — checks if userA follows userB
  - `addFollow()` / `removeFollow()` — manage follows bidirectionally
  - Increments/decrements `following_count` for the follower and `followers_count` for the followee
  - `followed_user_ids` field: semicolon-separated list of user IDs

- **Following & Followers Pages**: View a user's following list (`/following/<username>`) and followers list (`/followers/<username>`)
  - Requires authentication (login)
  - Renders `following.html` and `followers.html` templates

### 7. Academic Hubs System

The application features **12 academic hubs** — program-specific collaboration spaces:

```
Engineering & Technology, Computer Applications, Management & Business,
Commerce, Economics, Law, Humanities & Liberal Arts, Psychology,
Sciences, Design & Media, Allied Health Sciences, Doctoral Programs
```

Key functionality:
- **Hub Index** (`/hubs`) — lists all available academic hubs
- **Hub Page** (`/hub/<slug>`) — displays posts from a specific academic hub/programme
- **Access Control**:
  - Users can only see posts from their own programme by default
  - Admin users (`isAdminRole()`) have access to all hubs
  - Cross-programme access granted through approval system
- **Redirect Routes**: `/cse`, `/ece`, `/me` — shorthand redirects to hub pages

### 8. Access Request & Approval System (Admin-Only)

Users can request access to academic hubs outside their programme:

- **Request Access** (`POST /request-access`) — submit request to access a hub
  - Stores request in `access_requests.csv` with status "PENDING"
  - Records timestamp of request

- **Pending Requests** (`GET /pending-requests`) — admin-only dashboard
  - Lists all pending access requests
  - Only accessible to staff/admin users (`isStaffRole()`)
  - Renders `admin_requests.html`

- **Approve/Reject Request** (`POST /approve-request`, `POST /reject-request`) — admin actions
  - Updates request status to "APPROVED" or "REJECTED"
  - Records approver user ID and timestamp
  - Approved requests stored in `approved_access.csv`
  - Users with approved access can view hub posts

### 9. Role Normalization & Helper Functions

The codebase includes utility functions for role management:

- `normalizeRole(role)` — standardizes role strings (handles case variations)
- `isStaffRole(role)` — checks if role is "Staff" or "Admin"
- `isAdminRole(role)` — checks if role is "Admin"
- Role-based UI rendering: `is_user`, `is_prof`, `is_staff`, `is_admin` context flags

## Database Schema Extensions

### Users CSV (17 Columns)
```
ID, Handle, Email, FullName, Role, Programme, PasswordHash, Bio, IsVerified,
CreatedAt, Location, Link, FollowingCount, FollowersCount, Posts,
LikedPostIDs, FollowedUserIDs
```

### Posts CSV (8 Columns)
```
PostID, UserID, Content, ParentID, LikesCount, RetweetsCount, CreatedAt, Role
```

### Additional CSV Files
- `database/students.csv` — student posts (filtered view)
- `database/teachers.csv` — teacher posts (filtered view)
- `database/staff.csv` — staff posts (filtered view)
- `database/news.csv` — trending news sidebar
- `database/access_requests.csv` — hub access requests (ID, UserID, Programme, Status, CreatedAt, ApprovedBy)
- `database/approved_access.csv` — approved cross-programme access (UserID, AllowedProgrammes, ApprovedAt, ApprovedBy)

## Next Steps for Your Journey

1.  **Analyze `main.cpp` Thoroughly**: Review how `authenticateUser()` and `registerUser()` manipulate strings and interact with files. Notice error handling concepts like returning `-1` or `-2` for different failure states.
2.  **Understand Memory Management**: Notice how C++ standard library strings (`std::string`) are frequently passed by constant reference (e.g., `const string& val`) to getter and setter methods. This avoids the performance hit of copying large amounts of text in memory.
3.  **Study the Social Features**: The like/follow system demonstrates CSV-based relationship management without a relational database — observe how semicolon-separated lists encode relationships.
4.  **Try Extending It!**: Challenge yourself. Try adding a "Delete Post" feature (already implemented!) or build a "trending posts" system that shows the most-liked posts. You would need to read `posts.csv`, sort by likes, and render the top N posts to a new page.

Happy coding!

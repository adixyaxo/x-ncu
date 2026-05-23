<!-- Section 1 -->

# Section 1: Crow Framework & Application Setup

## Web Server Architecture (Lines 1-16, 859-862, 1594)

---

## What is the Crow Framework?

- **Crow** is a C++ micro web framework inspired by Python's Flask
- Provides HTTP routing, request/response handling, and template rendering
- Header-only library — just `#include "crow.h"`
- Supports **Mustache templates** for dynamic HTML rendering

---

## Application Initialization

```cpp
crow::SimpleApp app;
crow::mustache::set_base(".");
```

- `crow::SimpleApp` — creates a single-threaded HTTP application instance
- `crow::mustache::set_base(".")` — sets the root directory for HTML template files
- Server binds to `127.0.0.1:18080` (localhost)

---

## Key Libraries Used


| Library                | Purpose                                  |
| ---------------------- | ---------------------------------------- |
| `crow.h`               | Web framework (routing, HTTP, templates) |
| `jwt-cpp/jwt.h`        | JSON Web Token creation & verification   |
| `sha256.h`             | Password hashing using SHA-256           |
| `otp_service.h`        | OTP generation and email verification    |
| `<fstream>, <sstream>` | CSV file I/O and parsing                 |

---

## Routing Pattern — CROW_ROUTE Macro

```cpp
CROW_ROUTE(app, "/login")([](const crow::request &req) {
    auto page = crow::mustache::load("login.html");
    return crow::response(page.render());
});
```

- `CROW_ROUTE(app, "/path")` — registers a URL endpoint
- Lambda captures the incoming `crow::request`
- `.methods(crow::HTTPMethod::POST)` — restricts to specific HTTP methods
- Templates loaded with `crow::mustache::load()` and rendered with context data

---

## Mustache Templating Engine

```cpp
crow::mustache::context ctx;
ctx["title"] = "HOME | X-NCU";
ctx["user_name"] = currentUser.fullname();

auto page = crow::mustache::load("index.html");
return crow::response(page.render(ctx));
```

- Context object acts as a **key-value map** passed to HTML templates
- Supports strings, booleans, integers, and **vectors of contexts** (for loops)
- Templates use `{{variable}}` syntax for dynamic content injection

---

<!-- Section 2 -->

# Section 2: CSV-Based Database Layer

## Data Storage & Retrieval (Lines 88-210, 273-390, 468-709)

---

## Database Architecture — CSV Files

The application uses **flat CSV files** instead of a traditional database:


| CSV File                       | Purpose                         |
| ------------------------------ | ------------------------------- |
| `database/users.csv`           | User accounts (17 columns)      |
| `database/posts.csv`           | All posts/tweets                |
| `database/students.csv`        | Student-filtered posts          |
| `database/teachers.csv`        | Teacher-filtered posts          |
| `database/staff.csv`           | Staff-filtered posts            |
| `database/news.csv`            | Trending news items             |
| `database/access_requests.csv` | Hub access requests             |
| `database/approved_access.csv` | Approved cross-programme access |

---

## Users CSV Schema (17 Columns)

```
UserID, Handle, Email, FullName, Role, Programme, PasswordHash,
Bio, IsVerified, CreatedAt, Location, Link,
FollowingCount, FollowersCount, Posts,
LikedPostIDs, FollowedUserIDs
```

- Passwords stored as **SHA-256 hashes** — never in plaintext
- Handle auto-generated from email: `user@ncu.edu` → `@user`
- `IsVerified` stored as `"TRUE"` / `"FALSE"` string
- `LikedPostIDs` and `FollowedUserIDs` use semicolon-separated format

---

## CSV Read Pattern — User Constructor

```cpp
user::user(int targetUserNo) {
    ifstream file("database/users.csv");
    string line;
    getline(file, line);           // Skip header
    while (getline(file, line)) {
        stringstream ss(line);
        string idStr;
        getline(ss, idStr, ',');   // Extract column by comma
        if (stoi(idStr) == targetUserNo) {
            // Parse remaining columns...
            isFound_private = true;
            break;
        }
    }
}
```

- Opens CSV, skips header, iterates row-by-row
- Uses `stringstream` + `getline(ss, field, ',')` to split columns
- `try-catch` blocks guard against malformed data

---

## CSV Write Pattern — Register User

```cpp
ofstream outFile(filePath, ios::app);   // Append mode
outFile << newId << "," << handle << "," << email << ","
        << fullName << "," << role << "," << sha256(password) << ","
        << "" << "," << "FALSE" << "," << timestamp << ","
        << "" << "," << "" << "," << "0" << "," << "0" << "," << "" << "\n";
```

- **Auto-increment ID**: scans all rows to find `maxId`, then `newId = maxId + 1`
- **Newline safety**: checks if file ends with `\n` before appending
- Uses `ios::binary | ios::ate` + `seekg(-1)` to inspect last character

---

## CSV Update Pattern — Save Post

```cpp
void post::savepost(const post &p) {
    // 1. Read ALL lines into a vector<string>
    // 2. Find the matching row by ID
    // 3. Replace that row with updated data
    // 4. Rewrite the ENTIRE file
    ofstream outFile(filePath);
    for (const auto &l : lines)
        outFile << l << "\n";
}
```

- **Full-file rewrite** approach since CSV has no random-access update
- Reads into memory → modifies → writes back to disk
- Preserves header and all other rows unchanged

---

<!-- Section 3 -->

# Section 3: User & Post Data Models (OOP)

## Classes, Encapsulation & Data Sanitization (Lines 17-159, 392-624)

---

## User Class Design

```cpp
class user {
private:
    int id_private;
    string fullname_private, email_private, handle_private;
    string role_private, password_private, bio_private;
    bool is_verified_private, isFound_private;
    string location_private, link_private;
    string programme_private, liked_post_ids_private, followed_user_ids_private;
    int following_count_private, followers_count_private, posts_private;
public:
    // Overloaded getters & setters
    int id() const;         // getter
    void id(int val);       // setter
};
```

- **Encapsulation**: all data members are `private`
- **Overloaded naming**: same function name for get (`id()`) and set (`id(int)`)
- 17 fields matching the CSV columns exactly
- Includes social tracking: `programme`, `liked_post_ids`, `followed_user_ids`

---

## Post Class Design

```cpp
class post {
private:
    int id_private, user_id_private, parent_id_private;
    int likes_count_private, retweets_count_private;
    string content_private, created_at_private, role_private;
    bool isFound_private;
public:
    post();
    post(const string &content, int user_id, int parent_id = -1);
    static post getpost(int id);     // Static: fetch by ID
    static void savepost(const post &p);  // Static: update in CSV
    static bool deletepost(int post_id);  // Static: delete from CSV
};
```

- `parent_id = -1` means it's a **top-level post** (not a reply)
- `static` methods operate on the CSV file directly, not on an instance
- Constructor creates AND saves the post to CSV in one step
- `deletepost()` removes a post entirely from the database

---

## Input Sanitization (Post Content)

```cpp
content_private = content_input;
for (char &c : content_private) {
    if (c == '\n' || c == '\r')
        c = ' ';    // Prevent CSV row breaks
    else if (c == ',')
        c = ';';    // Prevent CSV column shifts
}
```

- **Critical for CSV integrity** — raw commas or newlines would corrupt the file
- Newlines (`\n`, `\r`) replaced with spaces
- Commas replaced with semicolons
- Applied before any file write operation

---

## Constructor Workflow — Creating a Post

1. **Sanitize** the input content (commas, newlines)
2. **Scan** `posts.csv` to find the highest existing ID
3. **Calculate** `newId = maxId + 1`
4. **Check** if file needs a trailing newline
5. **Lookup** the author's role via `user(user_id)`
6. **Write** the new row with timestamp to CSV

```
PostID, UserID, Content, ParentID, LikesCount, RetweetsCount, CreatedAt, Role
```

---

## Static Methods — getpost, savepost & deletepost

**`post::getpost(int id)`** — Read

- Opens `posts.csv`, scans for matching `PostID`
- Returns a fully populated `post` object with `isFound = true`
- Returns empty post with `isFound = false` if not found

**`post::savepost(const post &p)`** — Update

- Loads entire file into `vector<string>`
- Replaces the matching row with updated values
- Rewrites the complete file

**`post::deletepost(int post_id)`** — Delete

- Removes the post row from CSV
- Preserves all other rows
- Returns `true` on success, `false` if post not found

---

<!-- Section 4 -->

# Section 4: Authentication & Security

## JWT, SHA-256, OTP Verification (Lines 211-271, 714-857, 1293-1462)

---

## Authentication Flow Overview

```
User Login → Email + Password → SHA-256 Hash Check
    → Match Found → Generate JWT Token → Set Cookie
    → Redirect to Home Page
```

```
User Signup → Email + OTP Verification → Register in CSV
    → Generate JWT Token → Set Cookie → Redirect to Home
```

---

## Password Hashing — SHA-256

```cpp
if (storedPassword == sha256(inputPassword)) {
    return stoi(idStr);  // Authentication success
}
```

- `sha256()` from external `sha256.h` library
- Passwords **never stored in plaintext** — only the hash is saved
- During login, the input password is hashed and compared with stored hash
- One-way function: hash cannot be reversed to get the original password

---

## JWT Token — Creation

```cpp
std::string token = jwt::create()
    .set_issuer("x-ncu")
    .set_payload_claim("user_id", jwt::claim(std::to_string(userId)))
    .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours(24))
    .sign(jwt::algorithm::hs256{"meetthemakeraditya"});
```


| Field     | Value                  |
| --------- | ---------------------- |
| Issuer    | `x-ncu`                |
| Payload   | `user_id` (as string)  |
| Expiry    | 24 hours from creation |
| Algorithm | HMAC-SHA256            |

- Token set as **HttpOnly cookie** — not accessible via JavaScript (XSS protection)

---

## JWT Token — Verification

```cpp
int verify_token(const crow::request &req) {
    // 1. Extract token from Cookie header ("token=xxx")
    // 2. Fallback: check Authorization header ("Bearer xxx")
    // 3. Decode and verify:
    auto decoded = jwt::decode(token);
    jwt::verify()
        .allow_algorithm(jwt::algorithm::hs256{"meetthemakeraditya"})
        .with_issuer("x-ncu")
        .verify(decoded);
    // 4. Extract user_id from payload
    return std::stoi(decoded.get_payload_claim("user_id").as_string());
}
```

- Returns `user_id (> 0)` on success, `-1` on failure
- Called at the start of **every protected route**

---

## OTP-Based Email Verification

```cpp
// POST /send-otp — Generate and email an OTP
otp::OTPService::getInstance().generateAndSendOTP(email);

// POST /register — Verify the OTP before registration
if (!otp::OTPService::getInstance().verifyOTP(email, otp_val)) {
    return crow::response(401, "Invalid or Expired OTP");
}
```

- **Singleton pattern**: `OTPService::getInstance()`
- OTP sent to user's email before registration is allowed
- Prevents fake/spam account creation
- OTPs have an expiry window — expired OTPs are rejected

---

## Login vs Signup — Error Handling


| Scenario            | HTTP Code | Message                    |
| ------------------- | --------- | -------------------------- |
| Login success       | 303       | Redirect to`/`             |
| Wrong password      | 401       | "Wrong password"           |
| User not found      | 404       | "User Not Found"           |
| Database error      | 500       | "Could not open database"  |
| Signup: user exists | 422       | "User already exists"      |
| Signup: invalid OTP | 401       | "Invalid or Expired OTP"   |
| Missing fields      | 400       | "All fields are required!" |

---

<!-- Section 5 -->

# Section 5: API Routes & Feed Rendering

## Endpoints, Feed Logic & Profile System (Lines 859-1594)

---

## Route Map — Complete API (GET/POST)


| Method | Route                   | Description                 |
| ------ | ----------------------- | --------------------------- |
| GET    | `/`                     | Home feed (all posts)       |
| GET    | `/students`             | Student-only posts          |
| GET    | `/teachers`             | Teacher-only posts          |
| GET    | `/staff`                | Staff-only posts            |
| GET    | `/login`                | Login page                  |
| GET    | `/signup`               | Signup page                 |
| GET    | `/logout`               | Clear cookie, redirect      |
| GET    | `/profile/<handle>`     | User profile page           |
| GET    | `/editprofile`          | Edit profile form           |
| GET    | `/hubs`                 | Academic hubs index         |
| GET    | `/hub/<slug>`           | Single hub with posts       |
| GET    | `/following/<username>` | User's following list       |
| GET    | `/followers/<username>` | User's followers list       |
| GET    | `/pending-requests`     | Admin: view access requests |

---

## Route Map — POST/Action Endpoints


| Method | Route              | Description                          |
| ------ | ------------------ | ------------------------------------ |
| POST   | `/auth`            | Login authentication                 |
| POST   | `/register`        | New user registration                |
| POST   | `/send-otp`        | Send OTP to email                    |
| POST   | `/post`            | Create a new post                    |
| POST   | `/updatepost`      | Like (action=1) or repost (action=2) |
| POST   | `/deletepost`      | Delete a post (ownership required)   |
| POST   | `/updateprofile`   | Update profile fields                |
| POST   | `/like`            | Add like to post                     |
| POST   | `/unlike`          | Remove like from post                |
| POST   | `/follow`          | Follow a user                        |
| POST   | `/unfollow`        | Unfollow a user                      |
| POST   | `/request-access`  | Request hub access                   |
| POST   | `/approve-request` | Admin: approve access request        |
| POST   | `/reject-request`  | Admin: reject access request         |

---

## Home Feed Logic (`/`)

1. **Verify token** — redirect to `/login` if invalid
2. **Load current user** context (name, handle, initials)
3. **Read** all posts from `posts.csv`
4. **Count replies** using `unordered_map<int,int>` — maps `parent_id → count`
5. **Sort posts** newest-first by `post_id` (descending)
6. **Build context** for each post (author info, role badges, timestamps)
7. **Load news sidebar** from `news.csv`
8. **Render** `index.html` with all context data

---

## Role-Based Feed Filtering

```
/students  →  reads from database/students.csv
/teachers  →  reads from database/teachers.csv
/staff     →  reads from database/staff.csv
```

- Each route reads from a **different CSV file** pre-filtered by role
- Role badges displayed conditionally in templates:

```cpp
post_ctx["is_user"] = (role == "student" || role == "Student");
post_ctx["is_prof"] = (role == "teacher" || role == "Teacher");
post_ctx["is_staff"] = (role == "staff" || role == "Staff");
```

---

## Post Interactions — Like, Unlike, Retweet

```
POST /like?post_id=5        →  Add like to post #5
POST /unlike?post_id=5      →  Remove like from post #5
GET/POST /updatepost?action=1&id=5  →  Legacy: Like post #5
GET/POST /updatepost?action=2&id=5  →  Legacy: Retweet post #5
```

```cpp
// Like flow: Check if already liked, then add
if (hasUserLikedPost(user_id, postId))
    return crow::response(400, "Already liked");
addLikeToUser(user_id, postId);
p.likes_count(p.likes_count() + 1);
post::savepost(p);
```

- Prevents duplicate likes via `hasUserLikedPost()` check
- Updates post counter and user's liked list simultaneously

---

## Profile Page (`/profile/<handle>`)

- Accepts username as URL parameter (auto-prepends `@` if missing)
- Looks up user via `getuserprofile()` — searches by handle
- Renders profile data:
  - Name, handle, bio, location, link
  - Follower/following counts, post count
  - Join date (first 10 chars of `created_at`)
  - Verification badge status
- **Own profile detection**: `ctx["is_own_profile"] = true` if viewer = profile owner
- **Follow status**: `ctx["is_following"] = true` if viewer follows this user
- News sidebar loaded alongside profile

---

<!-- Section 6 NEW -->

# Section 6: Social Features & Relationships

## Likes, Follows, and User Lists (Lines 290-450)

---

## Like Helper Functions

```cpp
bool hasUserLikedPost(int userId, int postId);
void addLikeToUser(int userId, int postId);
void removeLikeFromUser(int userId, int postId);
```

- **Storage**: User's `liked_post_ids` field = semicolon-separated post IDs
- **Prevention**: `hasUserLikedPost()` prevents duplicate likes
- **API**: `/like` and `/unlike` routes use these helpers
- **Legacy**: `/updatepost?action=1` also supported for backwards compatibility

---

## Follow Helper Functions

```cpp
vector<int> getFollowedUsers(int userId);
bool doesUserFollow(int userA, int userB);
void addFollow(int userA, int userB);
void removeFollow(int userA, int userB);
```

- **Bidirectional Updates**: Both users' records updated (following_count, followers_count)
- **Storage**: User's `followed_user_ids` field = semicolon-separated user IDs
- **Validation**: `doesUserFollow()` checks relationships in O(n) time via CSV parsing
- **Self-follow Prevention**: `addFollow()` rejects if `userA == userB`

---

## Following & Followers List Routes

```cpp
GET /following/<username>  → Display all users that @username follows
GET /followers/<username>  → Display all users following @username
```

- **Following page**: Loads `followed_user_ids` from target user, builds context list
- **Followers page**: Scans entire users.csv, checks `doesUserFollow(checkId, targetUserId)`
- **Efficiency Note**: Followers scan is O(n*m) — could use inverse index
- Renders `following.html` and `followers.html` with user lists

---

## Follow Flow Example

```
User A clicks "Follow" on User B's profile
  → POST /follow?user_id=B
  → addFollow(A, B):
      - Append B to A's followed_user_ids
      - Increment A's following_count
      - Increment B's followers_count
      - updateUserInCSV() for both
  → Display "Following" badge on profile
```

---

<!-- Section 7 NEW -->

# Section 7: Academic Hubs System

## Programme-Specific Collaboration Spaces (Lines 1156-2744)

---

## Academic Hub Structure

```cpp
struct HubInfo {
    string slug;        // URL-safe identifier
    string name;        // Full name (e.g., "Engineering & Technology")
    string short_name;  // 2-3 letter code (e.g., "ET")
    string description; // Hub purpose
};
```

**12 Built-in Hubs:**

```
Engineering & Technology (ET)
Computer Applications (CA)
Management & Business (MBA)
Commerce (COM)
Economics (ECO)
Law (LAW)
Humanities & Liberal Arts (HLA)
Psychology (PSY)
Sciences (SCI)
Design & Media (DM)
Allied Health Sciences (AHS)
Doctoral Programs (PHD)
```

---

## Hub Slug Mapping

```cpp
string hubProgrammeFromSlug(const string &slug);
string hubSlugFromProgramme(const string &programme);
vector<crow::mustache::context> buildHubContexts(const string &currentProgramme = "");
```

- **Slug → Programme**: Convert URL slug to full programme name
- **Programme → Slug**: Reverse lookup for UI links
- **Alias Handling**: `/cse`, `/ece`, `/me` redirect to correct hub slugs

---

## Hub Index Route (`/hubs`)

```cpp
CROW_ROUTE(app, "/hubs")([](const crow::request &req) {
    // List all 12 academic hubs
    // Highlight user's current programme
    auto page = crow::mustache::load("hubs.html");
    return crow::response(page.render(ctx));
});
```

- Renders `hubs.html` with all hubs and current user's programme highlighted
- Entry point to browse and navigate programme communities

---

## Hub Page Route (`/hub/<slug>`)

```cpp
CROW_ROUTE(app, "/hub/<string>")([](const crow::request &req, string hubSlug) {
    string programme = hubProgrammeFromSlug(hubSlug);

    // Access control logic:
    bool hasAccess = isAdminRole(currentUser.role()) ||
                     currentUser.programme() == programme;

    // Check approved_access.csv for cross-programme approval
    // ...

    // If hasAccess, filter posts.csv by programme
    // ...
});
```

---

## Hub Access Control

**Default Access:**

- Users can view posts from their own programme
- Admins can access all hubs

**Cross-Programme Access:**

- Users can request access via `POST /request-access`
- Request stored in `access_requests.csv` with status "PENDING"
- Admin reviews in `/pending-requests` dashboard
- Admin approves via `POST /approve-request`
- Approved access stored in `approved_access.csv`

**Access Check Flow:**

```
1. Check if isAdminRole() → Grant access
2. Check if currentProgramme == hubProgramme → Grant access
3. Query approved_access.csv for user_id
4. If found and programme in AllowedProgrammes → Grant access
5. Otherwise → Show "needs_access" flag in template
```

---

## Access Request CSV Files

**access_requests.csv:**

```
RequestID, UserID, TargetProgramme, Status, CreatedAt, ApprovedByUserID
```

- Status: "PENDING", "APPROVED", or "REJECTED"
- ApprovedByUserID: Admin who approved (if applicable)

**approved_access.csv:**

```
UserID, AllowedProgrammes, ApprovedAt, ApprovedByUserID
```

- AllowedProgrammes: Semicolon-separated list (e.g., "Engineering & Technology;Law")
- Can be updated multiple times as user gains access to more hubs

---

## Admin Dashboard Routes

```cpp
GET /pending-requests      → List all pending access requests (admin-only)
POST /approve-request      → Approve a request (admin-only)
POST /reject-request       → Reject a request (admin-only)
```

- Only `isStaffRole()` can access (Staff and Admin)
- `renderadmin_requests.html` with request list
- Stores approval timestamp and approver user ID for audit trail

---

<!-- Section 8 NEW -->

# Section 8: Role-Based Access & Helper Functions

## Authorization Logic (Lines 1269-1286)

---

## Role Normalization & Checking

```cpp
string normalizeRole(const string &role);
bool isStaffRole(const string &role);
bool isAdminRole(const string &role);
```

**normalizeRole()**

- Converts "student" / "Student" → "Student"
- Converts "teacher" / "Teacher" → "Teacher"
- Converts "staff" / "Staff" → "Staff" or "Admin"

**isStaffRole()**

- Returns `true` if role is "Staff" or "Admin"
- Used for `/pending-requests`, `/approve-request`, `/reject-request`

**isAdminRole()**

- Returns `true` only if role is "Admin"
- Grants unlimited access to all hubs and admin features

---

## Context Building Helpers

```cpp
void addCurrentUserContext(crow::mustache::context &ctx, int user_id);
string getInitials(const string &name);
string getRequestParam(const crow::request &req, const string &name);
```

**addCurrentUserContext()**

- Populates ctx with user fields: name, handle, initials, programme
- Sets `is_staff` and `is_admin` flags
- Loads academic hubs list with current programme highlighted

**getInitials()**

- Extracts first letter + first letter of last name
- Falls back to "U" if name is empty

**getRequestParam()**

- Extracts query/body parameters
- Supports both GET (url_params) and POST (body params)

---

# Thank You!

## X-NCU — A Social Media Platform Built in C++

**Tech Stack Summary:**

- **Crow** — C++ web framework
- **CSV Files** — Flat-file database
- **JWT** — Token-based authentication
- **SHA-256** — Password hashing
- **OTP Service** — Email verification
- **Mustache** — HTML template engine
- **Social Features** — Likes, follows, user lists
- **Access Control** — Role-based hubs with approval system

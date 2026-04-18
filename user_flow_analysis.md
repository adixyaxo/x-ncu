# X-NCU Application Analysis: User Flow and Features

Based on the review of the C++ (Crow framework) backend and HTML frontend architecture, here is a detailed breakdown of the application's overall structure, feature set, and user navigation flow.

## 🌟 Application Overview
**X-NCU** is a social networking platform designed similarly to X (formerly Twitter). It focuses on an academic or institutional setting with distinct user roles: **Students**, **Teachers**, and **Staff**. The application relies on a basic CSV-backed database and uses JWT (JSON Web Tokens) for secure session management.

---

## 🗺️ Webpages and Routes (Features & Access)

### 1. Authentication Pages
- **Route: `/login`** (renders `login.html`)
  - **Features:** Allows existing users to sign in using their registered email and password.
  - **Access:** Only for unauthenticated users (logged-in users are redirected to the home feed).
- **Route: `/signup`** (renders `signup.html`)
  - **Features:** User registration containing a multi-step form: Name, Email, Password, and Role selection. It features **email OTP verification** via the `/send-otp` API to ensure valid setups before completion.
  - **Access:** Only for unauthenticated users.

### 2. Main Feed Pages (The Core Experience)
All main feeds render using `index.html`, dynamically injecting context based on the route.
- **Route: `/` (Home Feed)**
  - **Features:** The primary timeline. Displays a chronological stream of all global posts from the platform. It also displays a "Trends/News" sidebar dynamically loaded from `news.csv`.
- **Route: `/students` (Students Feed)**
  - **Features:** A scoped timeline displaying posts *only* authored by users with the "Student" role.
- **Route: `/teachers` (Teachers Feed)**
  - **Features:** A scoped timeline filtering posts *only* by "Teacher" accounts.
- **Route: `/staff` (Staff Feed)**
  - **Features:** A scoped timeline displaying posts *only* from institutional "Staff".

### 3. Profile Pages
- **Route: `/profile/<@username>`** (renders `profile.html`)
  - **Features:** Displays a user's public identity. Includes their Name, Handle (`@username`), Bio, verification status, join date, location, personal link, followers count, following count, and total posts. It contextually realizes if the viewing user owns the profile.
- **Route: `/editprofile`** (GET - renders `edit_profile.html`)
  - **Features:** A dedicated surface for users to update their profile. Form displays pre-filled data: Name, Bio, Location, and Website Link.
  - **Form Fields:** All profile information is auto-populated from the database before editing.
- **Route: `/updateprofile`** (POST - processes form submission)
  - **Features:** Backend endpoint that processes profile updates. Accepts POST requests with form data (fullname, bio, location, link).
  - **Validation:** Verifies user is logged in and updates their information in the CSV database.
  - **Redirect:** Upon success, redirects to the user's profile page (`/profile/@handle`).

### 4. Interactions and Actions (Invisible Routes)
- **Route: `/post` (POST)**: Endpoint handling the creation of new posts and replies (threads).
- **Route: `/updatepost` (GET/POST)**: Action endpoint for engaging with existing posts. `action=1` adds a **Like**, while `action=2` triggers a **Retweet**.
- **Route: `/deletepost` (GET/POST)**: Action endpoint for deleting posts. Only the post owner can delete their own posts.
  - **Query Parameter:** `id={post_id}` specifies which post to delete.
  - **Authorization:** Verifies the logged-in user is the post creator before allowing deletion.
  - **Response:** Returns 403 Forbidden if user is not the post owner. Returns 404 if post doesn't exist.
  - **Success:** Redirects to home feed (`/`) after deletion.
- **Route: `/logout`**: Clears the JWT session cookie and gracefully logs the user out.

---

## 🔄 The User Journey / Flow

### Phase 1: Onboarding
1. A new user lands on the base URL `/`. Being unauthenticated, the middleware intercepts and **redirects them to `/login`**.
2. Assuming they are brand new, they navigate to **`/signup`**.
3. They fill in their details and click to send an OTP to their email. 
4. The frontend pings the **`/send-otp`** endpoint. 
5. The user retrieves the code, inputs it, and submits the form to the **`/register`** route.
6. The backend validates the code and stores the user. It generates a JWT, sets it in a cookie, and redirects the user to the **Home feed (`/`)**.

### Phase 2: Engagement
1. The user, now authenticated, arrives at **`/`**. They can view the latest platform-wide interactions, verify their own handle/initials in the interface, and see trending news on the sidebar.
2. They draft their first thought in the compose box and hit post. The form submits to **`/post`** and then seamlessly refreshes/redirects back to the timeline, displaying their new post.
3. They browse their timeline and see a post from a professor. They click the heart icon, hitting **`/updatepost?action=1`**, incrementing the like counter.
4. Wishing to see only academic/teacher-related interactions, they click "Teachers" in the sidebar, navigating them to **`/teachers`**.

### Phase 3: Identity & Extensibility
1. They encounter an interesting user on the feed and click their handle. This routes them to **`/profile/@username`**.
2. They view the user's bio and follower count. 
3. Returning to their own profile, they decide to update their profile information and transition to **`/editprofile`** to modify their displayed data.
   - The form auto-populates with their current Name, Bio, Location, and Website Link.
   - They make changes and click "Save", submitting a POST request to **`/updateprofile`**.
   - The backend validates and updates their information in the database, then redirects back to their profile.
4. While browsing their posts, they decide to delete an old post. They click the delete button, which sends a request to **`/deletepost?id={post_id}`**.
   - The backend verifies they own the post and removes it from the database.
   - Upon successful deletion, they are redirected to the home feed.
5. Ending their session, they click the exit/logout button triggering **`/logout`**, destroying their session cookie and returning them to the **Login** screen.

---

## 📋 Complete Feature Set

### Authentication & User Management
- ✅ User Registration with Email OTP verification
- ✅ User Login with JWT token-based sessions
- ✅ User Logout with session clearing
- ✅ Role-based access (Student, Teacher, Staff)

### Profile Management
- ✅ View Public Profiles with user statistics
- ✅ Edit Profile with all fields pre-populated
- ✅ Update Profile Information (Name, Bio, Location, Website)
- ✅ Display Profile Metadata (Join date, Followers, Following, Posts count)

### Post Management
- ✅ Create Posts (with sanitization for CSV stability)
- ✅ View Posts on multiple feeds (Home, Students, Teachers, Staff)
- ✅ Like Posts (with like counter)
- ✅ Retweet Posts (with retweet counter)
- ✅ Delete Posts (with ownership verification)
- ✅ Reply/Thread Support (via parent_id relationships)

### Feed Management
- ✅ Home Feed (all posts from all users)
- ✅ Students-only Feed (filtered by role)
- ✅ Teachers-only Feed (filtered by role)
- ✅ Staff-only Feed (filtered by role)
- ✅ Trending News/Sidebar (dynamic content from news.csv)

---

## 🔐 Security Features

### Authorization
- ✅ JWT-based session management
- ✅ Login requirement for all authenticated routes
- ✅ Post deletion restricted to post owner
- ✅ Profile editing restricted to own profile

### Data Protection
- ✅ Password hashing using SHA256
- ✅ Email validation via OTP
- ✅ Input sanitization (CSV injection prevention)
- ✅ HTTPOnly cookies for JWT storage

---

## 🐛 Recent Fixes & Improvements

### Profile Update System (Fixed)
- **Issue:** Profile form using GET method, data not persisting
- **Fix:** Changed form to POST method, implemented proper `/updateprofile` endpoint
- **Result:** All profile fields (name, bio, location, link) now persist correctly

### Edit Profile Form (Fixed)
- **Issue:** Form displaying blank with no pre-filled data
- **Fix:** Added context variables for bio, location, link in `/editprofile` GET handler
- **Result:** Form auto-populates with user's current information

### Delete Post Feature (Implemented)
- **Route:** `/deletepost` with query parameter `id={post_id}`
- **Security:** Ownership verification - only post creator can delete
- **Validation:** Returns 403 Forbidden if user doesn't own the post
- **Result:** Users can now manage their own content

---

> [!NOTE]
> Database interactions across the ecosystem are handled predominantly by manipulating `.csv` files natively in C++. For instance, interactions (`likes`, `posts`, `students/teacher/staff feeds`) read and rewrite lines in real-time when actions occur.
>
> **Known Limitation:** The likes system currently has no duplicate-prevention mechanism. Users can like the same post multiple times. This can be addressed by implementing a `post_likes.csv` tracking system.

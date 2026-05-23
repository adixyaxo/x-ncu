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
- **Route: `/send-otp` (POST)**
  - **Features:** Generates and sends a one-time password (OTP) to the user's email address for verification during signup.
  - **Parameters:** `email` (required)

### 2. Main Feed Pages (The Core Experience)
All main feeds render using `index.html`, dynamically injecting context based on the route.
- **Route: `/` (Home Feed)**
  - **Features:** The primary timeline. Displays a chronological stream of all global posts from the platform (newest first). Shows "Trends/News" sidebar dynamically loaded from `news.csv`. Displays delete button for own posts.
  - **Interactions:** Users can like, retweet, and delete their own posts from this feed.
- **Route: `/students` (Students Feed)**
  - **Features:** A scoped timeline displaying posts *only* authored by users with the "Student" role. Shows deletion options for own posts.
- **Route: `/teachers` (Teachers Feed)**
  - **Features:** A scoped timeline filtering posts *only* by "Teacher" accounts. Shows deletion options for own posts.
- **Route: `/staff` (Staff Feed)**
  - **Features:** A scoped timeline displaying posts *only* from institutional "Staff". Shows deletion options for own posts.

### 3. Profile Pages
- **Route: `/profile/<@username>`** (renders `profile.html`)
  - **Features:** Displays a user's public identity with comprehensive profile data: Name, Handle (`@username`), Bio, verification status, join date, location, personal link, followers count, following count, and total posts count.
  - **Contextual Features:** Shows "Edit Profile" and "Message" buttons if viewing own profile. Shows "Follow/Unfollow" button if viewing another user's profile.
  - **User Posts:** Displays all posts authored by the profile user, newest first.
  - **Follow Integration:** Shows follow status and allows follow/unfollow actions.
- **Route: `/editprofile`** (GET - renders `edit_profile.html`)
  - **Features:** Dedicated form for users to update their profile. All fields are pre-populated with current data.
  - **Form Fields:** Name, Bio, Location, Website Link
  - **Access:** Only for logged-in users (viewing own profile)
- **Route: `/updateprofile`** (POST)
  - **Features:** Backend endpoint that processes profile updates.
  - **Parameters:** `fullname`, `bio`, `location`, `link`
  - **Validation:** Verifies user is logged in and updates their information in the CSV database.
  - **Redirect:** Upon success, redirects to the user's profile page (`/profile/@handle`).

### 4. Social Interaction Routes
- **Route: `/post` (POST)**: Endpoint for creating new posts and replies (threads).
  - **Parameters:** `content` (required), `parent_id` (optional, default -1 for top-level posts)
  - **Sanitization:** Content is sanitized to prevent CSV corruption (commas → semicolons, newlines → spaces)
  - **Success:** Redirects to home feed after post creation
- **Route: `/like` (POST)**: Add a like to a post.
  - **Parameters:** `post_id` (required)
  - **Validation:** Prevents duplicate likes, prevents liking own posts
  - **Storage:** Updates both post's `likes_count` and user's `liked_post_ids`
- **Route: `/unlike` (POST)**: Remove a like from a post.
  - **Parameters:** `post_id` (required)
  - **Validation:** Checks user has already liked the post
- **Route: `/updatepost` (GET/POST)**: Legacy like/repost endpoint.
  - **Parameters:** `action` (1=like, 2=retweet), `id` (post_id)
  - **Note:** Newer `/like` and `/unlike` routes are preferred
- **Route: `/deletepost` (GET/POST)**: Delete a post.
  - **Parameters:** `id` (post_id, required)
  - **Authorization:** Only post owner can delete their posts (returns 403 Forbidden otherwise)
  - **Response:** Returns 404 if post doesn't exist
  - **Success:** Redirects to home feed (`/`) after deletion
- **Route: `/logout`**: Clears the JWT session cookie and logs the user out.

### 5. Social Following Routes
- **Route: `/follow` (POST)**: Follow another user.
  - **Parameters:** `user_id` (required)
  - **Validation:** Cannot follow yourself, prevents duplicate follows
  - **Updates:** Increments follower's `following_count` and target user's `followers_count`
- **Route: `/unfollow` (POST)**: Unfollow a user.
  - **Parameters:** `user_id` (required)
  - **Validation:** Checks user is already following
- **Route: `/following/<username>`** (renders `following.html`)
  - **Features:** Displays a list of all users that `@username` is following.
  - **User Cards:** Shows follower name, handle, and bio.
  - **Access:** Requires authentication
- **Route: `/followers/<username>`** (renders `followers.html`)
  - **Features:** Displays all users who follow `@username`.
  - **User Cards:** Shows follower name, handle, and bio.
  - **Access:** Requires authentication

### 6. Academic Hubs (Programme-Based Collaboration Spaces)
- **Route: `/hubs`** (renders `hubs.html`)
  - **Features:** Index of all 12 academic hubs with descriptions. Highlights user's current programme.
  - **Access:** Only for logged-in users
  - **Hub List:** Engineering & Technology, Computer Applications, Management & Business, Commerce, Economics, Law, Humanities & Liberal Arts, Psychology, Sciences, Design & Media, Allied Health Sciences, Doctoral Programs
- **Route: `/hub/<slug>`** (renders `hub.html`)
  - **Features:** Programme-specific collaboration space showing posts from users in that programme.
  - **Access Control:**
    - Users can access their own programme hub by default
    - Admins have access to all hubs
    - Users can request access to other hubs (see below)
  - **Cross-Programme Access:** Shows "needs_access" message if user lacks access; allows requesting access
  - **Posts:** Displays posts filtered by programme (only shows posts from users whose programme matches)
- **Redirect Routes:** `/cse`, `/ece`, `/me`
  - **Features:** Shorthand URLs that redirect to the appropriate hub page
  - **Example:** `/cse` redirects to `/hub/computer-applications`

### 7. Hub Access Management (Admin-Only Features)
- **Route: `/request-access` (POST)**: Request access to a hub outside user's programme.
  - **Parameters:** `programme` (hub name or slug)
  - **Storage:** Creates entry in `access_requests.csv` with status "PENDING"
  - **Timestamp:** Records request creation time
- **Route: `/pending-requests`** (renders `admin_requests.html`)
  - **Features:** Admin dashboard showing all pending hub access requests.
  - **Access:** Only accessible to Staff and Admin users (`isStaffRole()`)
  - **Display:** Shows requester name, programme requested, and request details
  - **Actions:** Buttons to approve or reject each request
- **Route: `/approve-request` (POST)**: Admin approval of an access request.
  - **Parameters:** `request_id` (required)
  - **Updates:** Changes request status to "APPROVED", records approval timestamp and approver user ID
  - **Cross-Programme Access:** Adds entry to `approved_access.csv` granting user access to programme
  - **Access:** Only for Staff and Admin users
- **Route: `/reject-request` (POST)**: Admin rejection of an access request.
  - **Parameters:** `request_id` (required)
  - **Updates:** Changes request status to "REJECTED", records rejection timestamp and rejecting admin user ID
  - **Access:** Only for Staff and Admin users

---

## 🔄 The User Journey / Flow

### Phase 1: Onboarding
1. A new user lands on the base URL `/`. Being unauthenticated, the middleware intercepts and **redirects them to `/login`**.
2. Assuming they are brand new, they navigate to **`/signup`**.
3. They fill in their details (Name, Email, Password, Role/Programme) and click to send an OTP to their email.
4. The frontend pings the **`/send-otp`** endpoint.
5. They retrieve the code from their email, input it, and submit the form to the **`/register`** route.
6. The backend validates the OTP and stores the user in the database. It generates a JWT, sets it in a cookie, and redirects the user to the **Home feed (`/`)**.

### Phase 2: Engagement (Posting & Interactions)
1. The user, now authenticated, arrives at **`/`**. They can view the latest platform-wide posts, verify their own handle/initials in the interface, and see trending news on the sidebar.
2. They draft their first thought in the compose box and hit post. The form submits to **`/post`** and then seamlessly redirects back to the timeline, displaying their new post.
3. They browse their timeline and see a post from a professor. They click the heart icon, hitting **`/like?post_id={id}`**, which records the like and increments the like counter.
4. To unlike, they click the heart again, hitting **`/unlike?post_id={id}`**.
5. They see an interesting post and want to retweet it (legacy: **`/updatepost?action=2`**).
6. Wishing to see only academic/student-related interactions, they click "Students" in the sidebar, navigating them to **`/students`**.
7. While reading, they decide to delete an old post of their own. They click the delete button, which sends a request to **`/deletepost?id={post_id}`**.
   - The backend verifies they own the post and removes it from the database.
   - Upon successful deletion, they are redirected to the home feed.

### Phase 3: Social Interactions (Follows & Lists)
1. They encounter an interesting user on the feed and click their handle. This routes them to **`/profile/@username`**.
2. They view the user's bio, follower count, following count, and posts.
3. They click the "Follow" button, sending a **`POST /follow?user_id={id}`** request.
   - The backend increments both users' follower/following counts.
   - The user now sees a "Following" badge on the profile.
4. They click "View Following" to see who this user is following — navigating to **`/following/@username`**.
5. They click "View Followers" to see this user's followers — navigating to **`/followers/@username`**.

### Phase 4: Profile & Identity
1. Returning to their own profile, they decide to update their profile information and transition to **`/editprofile`** to modify their displayed data.
   - The form auto-populates with their current Name, Bio, Location, and Website Link.
   - They make changes and click "Save", submitting a POST request to **`/updateprofile`**.
   - The backend validates and updates their information in the database, then redirects back to their profile.
2. They navigate to **`/hubs`** to explore academic collaboration spaces.

### Phase 5: Academic Hubs & Cross-Programme Access
1. They browse the **`/hubs`** index and see all 12 available academic hubs. Their own programme (e.g., "Engineering & Technology") is highlighted.
2. They click on their hub to navigate to **`/hub/engineering-technology`**, seeing posts from other students and faculty in their programme.
3. They encounter a post from someone in the "Law" programme discussing a topic of interest. They want to join that hub.
4. They navigate to **`/hub/law`**. Since they're not in the Law programme, the page shows "Access Requested" message with a "Request Access" button.
5. They click "Request Access", sending a **`POST /request-access?programme=Law`** request.
   - The request is recorded in `access_requests.csv` with status "PENDING".
   - They see a confirmation message: "Access request submitted for review."
6. Later, an admin user logs in and navigates to **`/pending-requests`** (admin dashboard).
   - They see the pending request from the user wanting access to the Law hub.
   - They review the request and click **`/approve-request?request_id=5`**.
   - The request status changes to "APPROVED" and an entry is added to `approved_access.csv`.
7. The user returns to **`/hub/law`**. Since their access is now approved, they can see and interact with posts from the Law programme.

### Phase 6: Conclusion
1. They're satisfied with their session and want to log out.
2. They click the exit/logout button triggering **`/logout`**, destroying their session cookie and returning them to the **Login** screen.

---

## 📋 Complete Feature Set

## 📋 Complete Feature Set

### Authentication & User Management
- ✅ User Registration with Email OTP verification
- ✅ User Login with JWT token-based sessions (24-hour expiry)
- ✅ User Logout with session clearing (HttpOnly cookies)
- ✅ Role-based access (Student, Teacher, Staff, Admin)
- ✅ Programme/Hub assignment during signup

### Profile Management
- ✅ View Public Profiles with user statistics
- ✅ Edit Profile with all fields pre-populated
- ✅ Update Profile Information (Name, Bio, Location, Website)
- ✅ Display Profile Metadata (Join date, Followers, Following, Posts count)
- ✅ Profile verification badges and role indicators

### Post Management
- ✅ Create Posts (with sanitization for CSV stability)
- ✅ View Posts on multiple feeds (Home, Students, Teachers, Staff)
- ✅ Delete Posts (with ownership verification)
- ✅ Reply/Thread Support (via parent_id relationships)
- ✅ Post timestamps and role-based display

### Like & Interaction System
- ✅ Like Posts with duplicate-prevention
- ✅ Unlike Posts (remove like)
- ✅ Like Counter (incremented with each like)
- ✅ Retweet/Repost functionality
- ✅ Interaction counts displayed on each post
- ✅ Prevention of liking own posts

### Social Follow System
- ✅ Follow Other Users (bidirectional relationship tracking)
- ✅ Unfollow Users (with counter updates)
- ✅ Following Count (increment/decrement with follows)
- ✅ Followers Count (tracked for each user)
- ✅ View Following List (/following/<username>)
- ✅ View Followers List (/followers/<username>)
- ✅ Follow Status Display (shows "Following" badge on profiles)
- ✅ Prevention of following yourself

### Feed Management
- ✅ Home Feed (all posts from all users, newest first)
- ✅ Students-only Feed (filtered by role)
- ✅ Teachers-only Feed (filtered by role)
- ✅ Staff-only Feed (filtered by role)
- ✅ Trending News/Sidebar (dynamic content from news.csv)
- ✅ Reply Count Display (shows thread information)

### Academic Hubs System
- ✅ Hub Index (/hubs) displaying all 12 academic programmes
- ✅ Programme-specific Collaboration Spaces (/hub/<slug>)
- ✅ 12 Built-in Academic Hubs:
  - Engineering & Technology
  - Computer Applications
  - Management & Business Administration
  - Commerce
  - Economics
  - Law
  - Humanities & Liberal Arts
  - Psychology
  - Sciences
  - Design & Media
  - Allied Health Sciences
  - Doctoral (Ph.D.) Programs
- ✅ Hub-based Post Filtering (shows only posts from users in that programme)
- ✅ Shorthand Hub Routes (/cse, /ece, /me redirects)
- ✅ Current Programme Highlighting in hub list

### Hub Access Control & Approvals
- ✅ Default Access (users can access own programme hub)
- ✅ Admin Access (admins can access all hubs)
- ✅ Cross-Programme Access Requests (/request-access)
- ✅ Pending Request Dashboard (/pending-requests - admin only)
- ✅ Approve Access Requests (/approve-request - admin only)
- ✅ Reject Access Requests (/reject-request - admin only)
- ✅ Access Status Messages ("needs_access", "needs_approval", etc.)
- ✅ Approval Audit Trail (records approver and timestamp)
- ✅ Multiple Programme Access (users can be approved for multiple hubs)

### Role-Based Access Control
- ✅ Student Role (default programme access)
- ✅ Teacher Role (educator access)
- ✅ Staff Role (institutional staff access + admin capabilities)
- ✅ Admin Role (full system access)
- ✅ Role Normalization (handles case variations)
- ✅ Role Badges (displayed on posts and profiles)

---

## 🔐 Security Features

### Authorization
- ✅ JWT-based session management (24-hour expiry)
- ✅ Login requirement for all authenticated routes (verify_token on every protected route)
- ✅ Post deletion restricted to post owner (403 Forbidden if not owner)
- ✅ Profile editing restricted to own profile (401 Unauthorized if different user)
- ✅ Admin-only access to `/pending-requests`, `/approve-request`, `/reject-request`
- ✅ Staff-only visibility of admin features
- ✅ Cross-programme access control (programmes locked by default, approval required)

### Data Protection
- ✅ Password hashing using SHA256 (never stored in plaintext)
- ✅ Email validation via OTP (prevents fake accounts)
- ✅ Input sanitization (CSV injection prevention via comma/newline replacement)
- ✅ HTTPOnly cookies for JWT storage (XSS protection)
- ✅ Token verification with issuer checking (jwt::algorithm::hs256)
- ✅ Safe type conversion (try-catch blocks for stoi conversions)

### Data Persistence
- ✅ CSV-based data storage (flat files with proper escaping)
- ✅ Full-file rewrite on updates (ensures consistency)
- ✅ Newline safety checks (prevents file corruption)
- ✅ Auto-increment ID tracking (scans maxId before creating new records)

---

## 🐛 Recent Fixes & Improvements

### Social Interaction System (Implemented)
- **Like/Unlike System**: Users can like and unlike individual posts
  - Prevention of duplicate likes via `hasUserLikedPost()` check
  - Updates tracked in user's `liked_post_ids` field
  - Separate `/like` and `/unlike` routes for better semantics
  - Result: Full social engagement on posts

### Follow System (Implemented)
- **Follow/Unfollow**: Users can build social networks
  - Bidirectional counter updates (following_count, followers_count)
  - Following list tracking in user's `followed_user_ids` field
  - `/following/<username>` and `/followers/<username>` list pages
  - Result: Complete social graph implementation

### Academic Hubs Feature (Implemented)
- **Hub Architecture**: 12 academic programmes as collaboration spaces
  - Hub index with all programmes listed
  - Hub-specific feeds showing only programme members' posts
  - Access control preventing cross-programme visibility by default
  - Result: Academic community organization

### Hub Access Approval System (Implemented)
- **Request Flow**: Users can request access to other programmes' hubs
  - Admin dashboard to review pending requests
  - Approve/reject functionality with audit trail
  - Access status displayed on hub pages
  - Result: Controlled cross-programme collaboration

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
> Database interactions across the ecosystem are handled predominantly by manipulating `.csv` files natively in C++. For instance, interactions (`likes`, `follows`, `posts`, `students/teacher/staff feeds`, `access_requests`) read and rewrite lines in real-time when actions occur.
>
> **Current Implementation Note:** The system uses in-memory vectors and full-file rewrites for updates. This approach ensures data consistency but may have performance implications at scale (100K+ users). Production deployment would benefit from transitioning to a real database system like SQLite or PostgreSQL.
>
> **Known Limitations:**
> - Followers list requires O(n) scan of all users (could use inverse index)
> - Hub access checks scan approved_access.csv sequentially
> - No caching layer for frequently-accessed data (user profiles, posts)
> - Concurrent writes could cause race conditions (no file locking)

/*
 * OVERVIEW:
 * This is the main server file for the X-NCU application.
 * It handles all the website links (like home, profile, login).
 * It reads and writes data to simple CSV files instead of a big database.
 * It also uses tokens to keep users logged in safely.
 */
#include "crow.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <unordered_map>
#include <algorithm>
#include <jwt-cpp/jwt.h>
#include "sha256.h"
#include "otp_service.h"

using namespace std;

string normalizeRole(const string &role);
bool isStaffRole(const string &role);
bool isAdminRole(const string &role);

/*
 * USER CLASS:
 * This part defines what a "user" is in our system.
 * It holds details like their name, email, password, and followers.
 * It also has tools to save these details back into the users.csv file.
 */
class user
{
private:
    int id_private;
    string fullname_private = "";
    string email_private = "";
    string handle_private = "";
    string role_private = "";
    string password_private = "";
    string bio_private = "";
    string created_at_private = "";
    bool is_verified_private = false;
    bool isFound_private = false;

    // NEW VARIABLES
    string location_private = "";
    string link_private = "";
    int following_count_private = 0;
    int followers_count_private = 0;
    int posts_private = 0;

    // PROGRAMME & SOCIAL FEATURES
    string programme_private = "";
    string liked_post_ids_private = "";
    string followed_user_ids_private = "";

public:
    user();
    user(int targetUserNo);
    ~user();

    // ==========================================
    // GETTERS
    // ==========================================
    int id() const { return id_private; }
    string fullname() const { return fullname_private; }
    string email() const { return email_private; }
    string handle() const { return handle_private; }
    string role() const { return role_private; }
    string password() const { return password_private; }
    string bio() const { return bio_private; }
    bool is_verified() const { return is_verified_private; }
    bool isFound() const { return isFound_private; }
    int posts() const { return posts_private; }

    // NEW GETTERS
    string location() const { return location_private; }
    string link() const { return link_private; }
    int following_count() const { return following_count_private; }
    int followers_count() const { return followers_count_private; }
    string created_at() const { return created_at_private; }
    string programme() const { return programme_private; }
    string liked_post_ids() const { return liked_post_ids_private; }
    string followed_user_ids() const { return followed_user_ids_private; }
    // ==========================================
    // SETTERS
    // ==========================================
    void id(int val) { id_private = val; }
    void fullname(const string &val) { fullname_private = val; }
    void email(const string &val) { email_private = val; }
    void handle(const string &val) { handle_private = val; }
    void role(const string &val) { role_private = val; }
    void password(const string &val) { password_private = val; }
    void bio(const string &val) { bio_private = val; }
    void is_verified(bool val) { is_verified_private = val; }
    void isFound(bool val) { isFound_private = val; }
    void posts(int val) { posts_private = val; }

    // NEW SETTERS
    void location(const string &val) { location_private = val; }
    void link(const string &val) { link_private = val; }
    void following_count(int val) { following_count_private = val; }
    void followers_count(int val) { followers_count_private = val; }
    void created_at(const string &val) { created_at_private = val; }
    void programme(const string &val) { programme_private = val; }
    void liked_post_ids(const string &val) { liked_post_ids_private = val; }
    void followed_user_ids(const string &val) { followed_user_ids_private = val; }

    void updateUserInCSV();
};

user::user() {}

// Updated constructor to populate the private variables
user::user(int targetUserNo) // constructor -- read data from user.csv ya fir user database --- store krke ek class banaega
{
    ifstream file("database/users.csv");

    if (!file.is_open())
        return;

    string line;
    getline(file, line); // Skip header

    while (getline(file, line))
    {
        if (line.empty())
            continue; // Skip blank lines

        /*
         * BREAKING DOWN THE LINE:
         * We use a 'stringstream' to treat the text line like a mini-file.
         * Then, we use 'getline' to read word by word, stopping at every comma.
         * This is how we split the CSV row into separate pieces of data.
         */
        stringstream ss(line);
        string idStr;

        getline(ss, idStr, ',');

        if (idStr.empty())
            continue; // Skip if no ID is found

        /*
         * SAFE CONVERSION:
         * Since the ID from the file is text, we must convert it to a number using 'stoi'.
         * We use 'try-catch' because if the text is corrupted and isn't a number,
         * the program will crash. The 'catch' block stops the crash and just skips the bad user.
         */
        int current_id = -1;
        try
        {
            current_id = stoi(idStr);
        }
        catch (...)
        {
            std::cout << "[DEBUG] Skipped user. Bad idStr: '" << idStr << "' | Full line: " << line << "\n";
            continue;
        }

        if (current_id == targetUserNo)
        {
            id_private = targetUserNo;

            getline(ss, handle_private, ',');
            getline(ss, email_private, ',');
            getline(ss, fullname_private, ',');
            getline(ss, role_private, ',');
            getline(ss, programme_private, ',');
            getline(ss, password_private, ',');
            getline(ss, bio_private, ',');

            string verified_str;
            getline(ss, verified_str, ',');
            is_verified_private = (verified_str == "TRUE");

            getline(ss, created_at_private, ',');
            getline(ss, location_private, ',');
            getline(ss, link_private, ',');

            string following_str;
            getline(ss, following_str, ',');
            following_count_private = following_str.empty() ? 0 : stoi(following_str);

            string followers_str;
            getline(ss, followers_str, ',');
            followers_count_private = followers_str.empty() ? 0 : stoi(followers_str);

            string posts_str;
            getline(ss, posts_str, ',');
            posts_private = posts_str.empty() ? 0 : stoi(posts_str);

            getline(ss, liked_post_ids_private, ',');
            getline(ss, followed_user_ids_private);

            isFound_private = true;
            break;
        }
    }
}

user::~user() {}

void user::updateUserInCSV()
{
    ifstream file("database/users.csv");
    if (!file.is_open())
        return;

    /*
     * UPDATING A CSV FILE:
     * You cannot easily change a single line in the middle of a text file.
     * So, we read every single line into a list (vector) in memory.
     * When we find the user we want to update, we replace their old line with the new data.
     * Finally, we erase the old file and write the entire list back to it.
     */
    vector<string> lines;
    string line;
    getline(file, line);
    lines.push_back(line); // Header

    bool found = false;
    while (getline(file, line))
    {
        if (line.empty())
            continue;
        stringstream ss(line);
        string idStr;
        getline(ss, idStr, ',');

        if (!idStr.empty() && stoi(idStr) == id_private)
        {
            stringstream updated;
            updated << id_private << ","
                    << handle_private << ","
                    << email_private << ","
                    << fullname_private << ","
                    << role_private << ","
                    << programme_private << ","
                    << password_private << ","
                    << bio_private << ","
                    << (is_verified_private ? "TRUE" : "FALSE") << ","
                    << created_at_private << ","
                    << location_private << ","
                    << link_private << ","
                    << following_count_private << ","
                    << followers_count_private << ","
                    << posts_private << ","
                    << liked_post_ids_private << ","
                    << followed_user_ids_private;
            lines.push_back(updated.str());
            found = true;
        }
        else
        {
            lines.push_back(line);
        }
    }
    file.close();

    if (found)
    {
        ofstream outFile("database/users.csv");
        for (const auto &l : lines)
            outFile << l << "\n";
        outFile.close();
    }
}

// LIKE HELPER FUNCTIONS
bool hasUserLikedPost(int userId, int postId)
{
    user u(userId);
    if (!u.isFound()) return false;

    string likedIds = u.liked_post_ids();
    if (likedIds.empty()) return false;

    stringstream ss(likedIds);
    string id;
    while (getline(ss, id, ';'))
    {
        if (id == to_string(postId)) return true;
    }
    return false;
}

void addLikeToUser(int userId, int postId)
{
    user u(userId);
    if (!u.isFound()) return;

    if (hasUserLikedPost(userId, postId)) return;

    string likedIds = u.liked_post_ids();
    if (!likedIds.empty()) likedIds += ";";
    likedIds += to_string(postId);
    u.liked_post_ids(likedIds);
    u.updateUserInCSV();
}

void removeLikeFromUser(int userId, int postId)
{
    user u(userId);
    if (!u.isFound()) return;

    string likedIds = u.liked_post_ids();
    if (likedIds.empty()) return;

    stringstream ss(likedIds);
    string id;
    vector<string> remaining;
    string postIdStr = to_string(postId);

    while (getline(ss, id, ';'))
    {
        if (id != postIdStr) remaining.push_back(id);
    }

    string newLikedIds;
    for (size_t i = 0; i < remaining.size(); ++i)
    {
        if (i > 0) newLikedIds += ";";
        newLikedIds += remaining[i];
    }

    u.liked_post_ids(newLikedIds);
    u.updateUserInCSV();
}

// FOLLOW HELPER FUNCTIONS
vector<int> getFollowedUsers(int userId)
{
    vector<int> followed;
    user u(userId);
    if (!u.isFound()) return followed;

    string followedIds = u.followed_user_ids();
    if (followedIds.empty()) return followed;

    stringstream ss(followedIds);
    string id;
    while (getline(ss, id, ';'))
    {
        try { followed.push_back(stoi(id)); }
        catch (...) {}
    }
    return followed;
}

bool doesUserFollow(int userA, int userB)
{
    user u(userA);
    if (!u.isFound()) return false;

    string followedIds = u.followed_user_ids();
    if (followedIds.empty()) return false;

    stringstream ss(followedIds);
    string id;
    while (getline(ss, id, ';'))
    {
        if (id == to_string(userB)) return true;
    }
    return false;
}

void addFollow(int userA, int userB)
{
    if (userA == userB) return;
    if (doesUserFollow(userA, userB)) return;

    user u(userA);
    if (!u.isFound()) return;

    string followedIds = u.followed_user_ids();
    if (!followedIds.empty()) followedIds += ";";
    followedIds += to_string(userB);
    u.followed_user_ids(followedIds);
    u.following_count(u.following_count() + 1);
    u.updateUserInCSV();

    user targetUser(userB);
    if (targetUser.isFound())
    {
        targetUser.followers_count(targetUser.followers_count() + 1);
        targetUser.updateUserInCSV();
    }
}

void removeFollow(int userA, int userB)
{
    user u(userA);
    if (!u.isFound()) return;

    string followedIds = u.followed_user_ids();
    if (followedIds.empty()) return;

    stringstream ss(followedIds);
    string id;
    vector<string> remaining;
    string userBStr = to_string(userB);

    while (getline(ss, id, ';'))
    {
        if (id != userBStr) remaining.push_back(id);
    }

    string newFollowedIds;
    for (size_t i = 0; i < remaining.size(); ++i)
    {
        if (i > 0) newFollowedIds += ";";
        newFollowedIds += remaining[i];
    }

    u.followed_user_ids(newFollowedIds);
    u.following_count(u.following_count() - 1);
    u.updateUserInCSV();

    user targetUser(userB);
    if (targetUser.isFound())
    {
        targetUser.followers_count(targetUser.followers_count() - 1);
        targetUser.updateUserInCSV();
    }
}

/*
 * GET USER PROFILE:
 * This tool searches the users.csv file to find someone by their username.
 * If it finds them, it gives back their secret ID number.
 */
int getuserprofile(const string &username)
{
    ifstream file("database/users.csv");
    if (!file.is_open())
    {
        cerr << "Error: Could not open database/users.csv. Check your file paths." << endl;
        return -2;
    }

    string line;

    // 1. Skip the header row
    if (file.good())
    {
        getline(file, line);
    }

    // 2. Loop through the remaining rows
    while (getline(file, line))
    {
        if (line.empty())
            continue; // Skip any empty lines

        stringstream ss(line);
        string idStr, currentUsername;

        // 3. Extract Column 1 (UserID) and Column 2 (Username)
        getline(ss, idStr, ',');
        getline(ss, currentUsername, ',');

        // 4. Check if we have a match
        if (currentUsername == username)
        {
            file.close();
            try
            {
                return stoi(idStr); // Convert the ID string to an integer and return it
            }
            catch (...)
            {
                return -1; // Safety catch in case the ID in the CSV is corrupted/not a number
            }
        }
    }

    file.close();

    // 5. If the loop finishes without returning, the user wasn't found
    return -1;
}

/*
 * AUTHENTICATE USER:
 * This checks if the email and password entered match our records.
 * It reads the users.csv file and compares passwords safely.
 * If correct, it lets the user log in.
 */
int authenticateUser(const string &inputEmail, const string &inputPassword)
{
    // 1. Open the CSV file
    ifstream file("database/users.csv");

    if (!file.is_open())
    {
        cerr << "Error: Could not open database/users.csv. Check your file paths." << endl;
        return -2;
    }

    string line;

    // 2. Read and discard the first line (the header row)
    getline(file, line);

    // 3. Loop through the remaining rows one by one
    while (getline(file, line))
    {
        stringstream ss(line);
        string idStr, email, fullName, handle, role, programme, storedPassword, is_verified, bio, created_at;
        string location, link, following, followers, posts;

        // 4. Extract each column up to the comma
        getline(ss, idStr, ',');
        getline(ss, handle, ',');
        getline(ss, email, ',');
        getline(ss, fullName, ',');
        getline(ss, role, ',');
        getline(ss, programme, ',');
        getline(ss, storedPassword, ',');
        getline(ss, bio, ',');
        getline(ss, is_verified, ',');
        getline(ss, created_at, ',');

        getline(ss, location, ',');
        getline(ss, link, ',');
        getline(ss, following, ',');
        getline(ss, followers, ',');
        string posts_str;
        getline(ss, posts_str);

        // 5. Check for a match
        if (email == inputEmail)
        {
            if (storedPassword == sha256(inputPassword))
            {
                // Match found! Convert the string ID to an integer and return it
                return stoi(idStr);
            }
            else
            {
                // Email found, but password doesn't match
                return -1;
            }
        }
    }

    // 6. Loop finished, email was never found
    return 0; // Return 0 here to trigger your "User Not Found" response
}

/*
 * REGISTER USER:
 * This adds a brand new user to our system.
 * It creates a new row in the users.csv file with all their details.
 * It also makes sure their email is not already taken.
 */
int registerUser(const string &email, const string &fullName, const string &role, const string &password)
{
    string filePath = "database/users.csv";
    ifstream inFile(filePath);

    int maxId = 0;

    if (inFile.is_open())
    {
        string line;
        getline(inFile, line);

        while (getline(inFile, line))
        {
            if (line.empty())
                continue;

            stringstream ss(line);
            string idStr, storedEmail, dummy;

            getline(ss, idStr, ',');
            getline(ss, dummy, ',');       // Username
            getline(ss, storedEmail, ','); // Email

            if (storedEmail == email)
            {
                inFile.close();
                return -1; // User Already Exists
            }

            try
            {
                int currentId = stoi(idStr);
                if (currentId > maxId)
                {
                    maxId = currentId;
                }
            }
            catch (...)
            {
            }
        }
        inFile.close();
    }

    int newId = maxId + 1;

    string handle = "";
    size_t atPos = email.find('@');
    if (atPos != string::npos)
    {
        handle = "@" + email.substr(0, atPos);
    }
    else
    {
        handle = "@user" + to_string(newId);
    }

    bool needsNewline = false;
    {
        ifstream checkFile(filePath, ios::binary | ios::ate);
        if (checkFile.is_open())
        {
            if (checkFile.tellg() > 0)
            {
                checkFile.seekg(-1, ios::end);
                char c;
                checkFile.get(c);
                if (c != '\n')
                {
                    needsNewline = true;
                }
            }
        }
    }

    ofstream outFile(filePath, ios::app);
    if (!outFile.is_open())
    {
        cerr << "Error: Could not open database/users.csv for writing." << endl;
        return -2;
    }

    if (needsNewline)
    {
        outFile << "\n";
    }
    auto t = std::time(nullptr);

    // 2. Convert to local time
    auto tm = *std::localtime(&t);

    // 3. Format the time into a stringstream
    std::ostringstream oss;

    // "%d/%m/%Y" gives you DD/MM/YYYY.
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");

    // Updated to write the complete row maintaining the 14 columns
    outFile << newId << ","
            << handle << ","
            << email << ","
            << fullName << ","
            << normalizeRole(role) << ","
            << "General" << ","
            << sha256(password) << ","
            << "" << ","        // Bio
            << "FALSE" << ","   // IsVerified
            << oss.str() << "," // CreatedAt
            << "" << ","        // Location
            << "" << ","        // Link
            << "0" << ","       // FollowingCount
            << "0" << ","       // FollowersCount
            << "0" << ","       // Posts
            << "" << ","        // LikedPostIDs
            << "" << "\n";      // FollowedUserIDs

    outFile.close();

    return newId;
}

/*
 * POST CLASS:
 * This part manages all the posts (like tweets) made by users.
 * It keeps track of who posted it, what it says, and how many likes it has.
 * It also saves new posts into the posts.csv file.
 */
class post
{
private:
    // PostID,UserID,Content,ParentID,LikesCount,RetweetsCount,CreatedAt,role
    int id_private;
    int user_id_private = -1;
    string content_private;
    int parent_id_private;
    int likes_count_private;
    int retweets_count_private;
    string created_at_private;
    string role_private;
    bool isFound_private;

public:
    // GETTERS AND SETTERS
    // GETTERS
    int id() const { return id_private; }
    int user_id() const { return user_id_private; }
    string content() const { return content_private; }
    int parent_id() const { return parent_id_private; }
    int likes_count() const { return likes_count_private; }
    int retweets_count() const { return retweets_count_private; }
    string created_at() const { return created_at_private; }
    string role() const { return role_private; }
    bool isFound() const { return isFound_private; }
    // SETTERS
    void id(int val) { id_private = val; }
    void user_id(int val) { user_id_private = val; }
    void content(const string &val) { content_private = val; }
    void parent_id(int val) { parent_id_private = val; }
    void likes_count(int val) { likes_count_private = val; }
    void retweets_count(int val) { retweets_count_private = val; }
    void created_at(const string &val) { created_at_private = val; }
    void role(const string &val) { role_private = val; }
    void isFound(bool val) { isFound_private = val; }
    // CONSTRUCTOR AND DESTRUCTOR
    post();
    post(const string &content, int user_id, int parent_id = -1);
    ~post();
    static post getpost(int id);
    static void savepost(const post &p);
    static bool deletepost(int post_id);
};

post::post()
{
}

post::~post()
{
}

post::post(const string &content_input, int user_id, int parent_id)
{
    id_private = 0;
    user_id_private = user_id;
    parent_id_private = parent_id;
    likes_count_private = 0;
    retweets_count_private = 0;
    created_at_private = "";
    role_private = "";
    isFound_private = false;

    /*
     * FIXING BAD INPUT (SANITIZATION):
     * A CSV file uses newlines for new rows and commas for new columns.
     * If a user types a comma or presses enter in their post, it will completely break our database!
     * To fix this, we replace all enter keys with a space, and all commas with a semicolon.
     */
    // 1. SANITIZE THE INPUT (Crucial for CSV stability)
    content_private = content_input;
    for (char &c : content_private)
    {
        if (c == '\n' || c == '\r')
        {
            c = ' '; // Replace newlines with spaces to prevent row breaks
        }
        else if (c == ',')
        {
            c = ';'; // Replace commas with semicolons to prevent column shifts
        }
    }

    string filePath = "database/posts.csv";
    ifstream inFile(filePath);

    /*
     * FINDING THE NEXT ID:
     * To give this new post a unique ID, we read the entire file to find the biggest ID currently used.
     * Then, we just add 1 to it. This ensures no two posts have the same number.
     */
    int maxId = 0;

    if (inFile.is_open())
    {
        string line;
        getline(inFile, line);

        while (getline(inFile, line))
        {
            if (line.empty())
                continue;

            stringstream ss(line);
            string idStr, dummy;

            getline(ss, idStr, ',');
            // We just need the ID to find the max, no need to parse the rest
            try
            {
                int currentId = stoi(idStr);
                if (currentId > maxId)
                {
                    maxId = currentId;
                }
            }
            catch (...)
            {
            }
        }
        inFile.close();
    }

    int newId = maxId + 1;

    // 2. ENSURE FILE ENDS WITH A NEWLINE
    bool needsNewline = false;
    {
        ifstream checkFile(filePath, ios::binary | ios::ate);
        if (checkFile.is_open() && checkFile.tellg() > 0)
        {
            checkFile.seekg(-1, ios::end);
            char c;
            checkFile.get(c);
            if (c != '\n')
            {
                needsNewline = true;
            }
        }
    }

    ofstream outFile(filePath, ios::app);
    if (!outFile.is_open())
    {
        cerr << "Error: Could not open database/posts.csv for writing." << endl;
        return;
    }

    if (needsNewline)
    {
        outFile << "\n";
    }

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d/%m/%Y");

    user author(user_id_private);
    string role = author.role().empty() ? "Student" : author.role();

    // 3. WRITE THE SANITIZED CONTENT
    outFile << newId << ","
            << user_id_private << ","
            << content_private << "," // Using the cleaned variable
            << parent_id << ","
            << "0" << ","       // LikesCount
            << "0" << ","       // RetweetsCount
            << oss.str() << "," // CreatedAt
            << role << "\n";

    outFile.close();
}

post post::getpost(int id)
{
    post p;
    ifstream file("database/posts.csv");

    if (!file.is_open())
    {
        cerr << "Error opening posts.csv\n";
        p.isFound(false);
        return p;
    }

    string line;
    getline(file, line); // skip header

    while (getline(file, line))
    {
        if (line.empty())
            continue;

        stringstream ss(line);

        string post_id_str;
        string user_id_str;
        string content;
        string parent_id_str;
        string likes_str;
        string reposts_str;
        string created_at;
        string role;

        getline(ss, post_id_str, ',');
        getline(ss, user_id_str, ',');
        getline(ss, content, ',');
        getline(ss, parent_id_str, ',');
        getline(ss, likes_str, ',');
        getline(ss, reposts_str, ',');
        getline(ss, created_at, ',');
        getline(ss, role);

        try
        {
            int post_id = stoi(post_id_str);

            if (post_id == id)
            {
                p.id(post_id);
                p.user_id(stoi(user_id_str));
                p.content(content);
                p.parent_id(stoi(parent_id_str));
                p.likes_count(stoi(likes_str));
                p.retweets_count(stoi(reposts_str));
                p.created_at(created_at);
                p.role(role);
                p.isFound(true);

                file.close();
                return p;
            }
        }
        catch (...)
        {
            continue;
        }
    }

    file.close();
    p.isFound(false);
    return p;
}

void post::savepost(const post &p)
{
    string filePath = "database/posts.csv";

    ifstream inFile(filePath);
    if (!inFile.is_open())
    {
        cerr << "Error: Could not open posts.csv\n";
        return;
    }

    vector<string> lines;
    string line;

    bool found = false;

    // Save header
    getline(inFile, line);
    lines.push_back(line);

    while (getline(inFile, line))
    {
        if (line.empty())
        {
            lines.push_back(line);
            continue;
        }

        stringstream ss(line);
        string idStr;

        getline(ss, idStr, ',');

        try
        {
            int currentId = stoi(idStr);

            if (currentId == p.id())
            {
                // Replace with new post data
                stringstream newLine;

                newLine << p.id() << ","
                        << p.user_id() << ","
                        << p.content() << ","
                        << p.parent_id() << ","
                        << p.likes_count() << ","
                        << p.retweets_count() << ","
                        << p.created_at() << ","
                        << p.role();

                lines.push_back(newLine.str());

                found = true;
            }
            else
            {
                lines.push_back(line);
            }
        }
        catch (...)
        {
            lines.push_back(line);
        }
    }

    inFile.close();

    if (!found)
    {
        cerr << "Post ID not found\n";
        return;
    }

    // Rewrite the entire file
    ofstream outFile(filePath);

    for (const auto &l : lines)
    {
        outFile << l << "\n";
    }

    outFile.close();
}

bool post::deletepost(int post_id)
{
    string filePath = "database/posts.csv";

    ifstream inFile(filePath);
    if (!inFile.is_open())
    {
        cerr << "Error: Could not open posts.csv\n";
        return false;
    }

    vector<string> lines;
    string line;
    bool found = false;

    // Save header
    getline(inFile, line);
    lines.push_back(line);

    while (getline(inFile, line))
    {
        if (line.empty())
        {
            continue;
        }

        stringstream ss(line);
        string idStr;

        getline(ss, idStr, ',');

        try
        {
            int currentId = stoi(idStr);

            if (currentId == post_id)
            {
                // Skip this post (delete it)
                found = true;
            }
            else
            {
                lines.push_back(line);
            }
        }
        catch (...)
        {
            lines.push_back(line);
        }
    }

    inFile.close();

    if (!found)
    {
        cerr << "Post ID not found\n";
        return false;
    }

    // Rewrite the entire file without the deleted post
    ofstream outFile(filePath);

    for (const auto &l : lines)
    {
        outFile << l << "\n";
    }

    outFile.close();
    return true;
}

//=================================
// CROW FUNCTIONS
//=================================

int verify_token(const crow::request &req);

// Helper function to require login
crow::response requireLogin(const crow::request &req)
{
    int userID = verify_token(req);
    if (userID <= 0)
    {
        crow::response res;
        res.code = 303;
        res.set_header("Location", "/login");
        return res;
    }

    crow::response res;
    res.code = 500;
    res.body = "Invalid login state.";
    return res;
}

// Generating user credentials
string getInitials(const string &name)
{
    if (name.empty())
        return "U";

    string initials;
    initials += name[0];

    size_t space = name.find(' ');
    if (space != string::npos && space + 1 < name.size())
        initials += name[space + 1];

    return initials;
}

struct HubInfo
{
    string slug;
    string name;
    string short_name;
    string description;
};

vector<HubInfo> getAcademicHubs()
{
    return {
        {"engineering-technology", "Engineering & Technology", "ET", "Engineering projects, labs, hardware, and technical collaboration."},
        {"computer-applications", "Computer Applications", "CA", "Software, systems, data, and applied computing discussions."},
        {"management-business-administration", "Management & Business Administration", "MBA", "Business, management, events, and campus leadership."},
        {"commerce", "Commerce", "COM", "Accounting, finance, markets, and commerce conversations."},
        {"economics", "Economics", "ECO", "Policy, markets, development, and economic research discussions."},
        {"law", "Law", "LAW", "Moot courts, case analysis, legal research, and policy debate."},
        {"humanities-liberal-arts", "Humanities & Liberal Arts", "HLA", "Culture, writing, history, society, and interdisciplinary thinking."},
        {"psychology", "Psychology", "PSY", "Behavior, cognition, counseling, and mental health discussions."},
        {"sciences", "Sciences", "SCI", "Physics, chemistry, biology, mathematics, and research collaboration."},
        {"design-media", "Design & Media", "DM", "Visual design, communication, content, and creative production."},
        {"allied-health-sciences", "Allied Health Sciences", "AHS", "Health sciences, clinical practice, wellness, and public health."},
        {"doctoral-phd-programs", "Doctoral (Ph.D.) Programs", "PHD", "Research methods, publications, thesis work, and doctoral life."}
    };
}

string hubProgrammeFromSlug(const string &slug)
{
    if (slug == "cse" || slug == "ca" || slug == "computer-apps") return "Computer Applications";
    if (slug == "ece" || slug == "me" || slug == "et" || slug == "engineering") return "Engineering & Technology";
    if (slug == "mba" || slug == "management") return "Management & Business Administration";
    if (slug == "humanities" || slug == "liberal-arts") return "Humanities & Liberal Arts";
    if (slug == "design") return "Design & Media";
    if (slug == "health" || slug == "allied-health") return "Allied Health Sciences";
    if (slug == "phd" || slug == "doctoral") return "Doctoral (Ph.D.) Programs";

    for (const auto &hub : getAcademicHubs())
    {
        if (hub.slug == slug || hub.name == slug)
            return hub.name;
    }

    return "";
}

string hubSlugFromProgramme(const string &programme)
{
    for (const auto &hub : getAcademicHubs())
    {
        if (hub.name == programme)
            return hub.slug;
    }

    return "";
}

vector<crow::mustache::context> buildHubContexts(const string &currentProgramme = "")
{
    vector<crow::mustache::context> hubs;
    for (const auto &hub : getAcademicHubs())
    {
        crow::mustache::context hub_ctx;
        hub_ctx["slug"] = hub.slug;
        hub_ctx["name"] = hub.name;
        hub_ctx["short_name"] = hub.short_name;
        hub_ctx["description"] = hub.description;
        if (hub.name == currentProgramme)
            hub_ctx["is_current"] = true;
        hubs.push_back(hub_ctx);
    }
    return hubs;
}

void addCurrentUserContext(crow::mustache::context &ctx, int user_id)
{
    user currentUser(user_id);

    if (!currentUser.isFound())
        return;

    string initials = "U";

    if (!currentUser.fullname().empty())
    {
        initials = "";
        initials += currentUser.fullname()[0];

        size_t space = currentUser.fullname().find(' ');
        if (space != string::npos && space + 1 < currentUser.fullname().size())
            initials += currentUser.fullname()[space + 1];
    }

    ctx["user_initials"] = initials;
    ctx["user_name"] = currentUser.fullname();
    ctx["user_handle"] = currentUser.handle();
    ctx["user_programme"] = currentUser.programme();
    ctx["user_hub_slug"] = hubSlugFromProgramme(currentUser.programme());
    ctx["is_staff"] = isStaffRole(currentUser.role());
    ctx["is_admin"] = isAdminRole(currentUser.role());
    ctx["academic_hubs"] = buildHubContexts(currentUser.programme());
}

string getRequestParam(const crow::request &req, const string &name)
{
    const char *urlValue = req.url_params.get(name);
    if (urlValue)
        return urlValue;

    crow::query_string params("?" + req.body);
    const char *value = params.get(name);
    return value ? value : "";
}

string normalizeRole(const string &role)
{
    if (role == "student" || role == "Student") return "Student";
    if (role == "teacher" || role == "Teacher") return "Teacher";
    if (role == "staff" || role == "Staff") return "Staff";
    if (role == "admin" || role == "Admin") return "Admin";
    return role;
}

bool isStaffRole(const string &role)
{
    return role == "staff" || role == "Staff" || isAdminRole(role);
}

bool isAdminRole(const string &role)
{
    return role == "admin" || role == "Admin";
}

vector<crow::mustache::context> loadNews()
{
    vector<crow::mustache::context> news_vector;

    ifstream news_file("database/news.csv");
    string line;

    if (news_file.good())
        getline(news_file, line);

    while (getline(news_file, line))
    {
        stringstream ss(line);

        string id, headline, category, time_ago, post_count;

        getline(ss, id, ',');
        getline(ss, headline, ',');
        getline(ss, category, ',');
        getline(ss, time_ago, ',');
        getline(ss, post_count, ',');

        crow::mustache::context news_ctx;

        news_ctx["headline"] = headline;
        news_ctx["category"] = category;
        news_ctx["time_ago"] = time_ago;
        news_ctx["post_count"] = post_count;

        news_vector.push_back(news_ctx);
    }

    return news_vector;
}

// verify function

/*
 * VERIFY TOKEN:
 * This checks the secret digital badge (token) of the user.
 * It makes sure the user is actually logged in before letting them see private pages.
 */
int verify_token(const crow::request &req)
{
    std::string token;
    auto cookie_header = req.get_header_value("Cookie");
    size_t pos = cookie_header.find("token=");

    if (pos != std::string::npos)
    {
        size_t end = cookie_header.find(';', pos);
        if (end == std::string::npos)
            end = cookie_header.length();
        token = cookie_header.substr(pos + 6, end - (pos + 6));
    }
    else
    {
        auto auth_header = req.get_header_value("Authorization");
        if (auth_header.size() >= 8 && auth_header.substr(0, 7) == "Bearer ")
        {
            token = auth_header.substr(7);
        }
        else
        {
            return -1;
        }
    }

    try
    {
        /*
         * CHECKING THE TOKEN'S MATH:
         * This uses a complex math formula (algorithm) and a secret password
         * to check if the token is real or fake.
         * If someone tried to make their own fake token, the math will fail and we reject them.
         * If the math is correct, we read the user's ID hidden inside the token and return it.
         */
        auto decoded = jwt::decode(token);

        jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{"meetthemakeraditya"})
            .with_issuer("x-ncu")
            .verify(decoded);

        // safer check
        if (!decoded.has_payload_claim("user_id"))
            return -1;

        return std::stoi(decoded.get_payload_claim("user_id").as_string());
    }
    catch (const std::exception &e)
    {
        return -1;
    }
}

/*
 * MAIN FUNCTION:
 * This is the starting point of the whole website.
 * It turns on the server and lists all the web pages users can visit.
 */
int main()
{
    crow::SimpleApp app;
    crow::mustache::set_base(".");

    // ==========================================
    // 1. HOME ROUTE
    // ==========================================
    /*
     * HOME PAGE ROUTE (/):
     * This prepares the main feed where you see all posts.
     * It checks if you are logged in, loads posts from the database, and sends them to the webpage.
     */
    CROW_ROUTE(app, "/")([](const crow::request &req)
                         {
    if (verify_token(req)<= 0) return requireLogin(req);

    crow::mustache::context ctx;
    ctx["title"] = "HOME | X-NCU";

    addCurrentUserContext(ctx, verify_token(req));

    std::vector<crow::mustache::context> posts_vector;

    std::ifstream posts_file("database/posts.csv");
    std::string line;

    if (!posts_file.is_open())
        return crow::response(500,"Could not open posts database");

    std::getline(posts_file,line); // skip header

    struct PostData
    {
        int post_id;
        int user_id;
        std::string content;
        int parent_id;
        int likes;
        int reposts;
        std::string created_at;
        std::string role;
    };

    std::vector<PostData> posts;

    while(std::getline(posts_file,line))
    {
        if(line.empty()) continue;

        std::stringstream ss(line);

        std::string post_id_str;
        std::string user_id_str;
        std::string content;
        std::string parent_id_str;
        std::string likes_str;
        std::string reposts_str;
        std::string created_at;
        std::string role;

        std::getline(ss,post_id_str,',');
        std::getline(ss,user_id_str,',');
        std::getline(ss,content,',');
        std::getline(ss,parent_id_str,',');
        std::getline(ss,likes_str,',');
        std::getline(ss,reposts_str,',');
        std::getline(ss,created_at,',');
        std::getline(ss,role);

        try
        {
            PostData p;

            p.post_id = std::stoi(post_id_str);
            p.user_id = std::stoi(user_id_str);
            p.content = content;
            p.parent_id = std::stoi(parent_id_str);
            p.likes = std::stoi(likes_str);
            p.reposts = std::stoi(reposts_str);
            p.created_at = created_at;
            p.role = role;

            posts.push_back(p);
        }
        catch(...)
        {
            continue;
        }
    }

    posts_file.close();

    // Count replies
    std::unordered_map<int,int> reply_count;

    for(auto &p : posts)
    {
        if(p.parent_id >= 0)
            reply_count[p.parent_id]++;
    }

    /*
     * SORTING POSTS:
     * This takes all the posts we loaded into memory and sorts them.
     * It compares the IDs (which go up automatically) to put the highest ID (newest post) at the top of the list.
     */
    // Sort newest first
    std::sort(posts.begin(),posts.end(),
        [](const PostData &a,const PostData &b)
        {
            return a.post_id > b.post_id;
        });

    for(auto &p : posts)
    {
        crow::mustache::context post_ctx;

        user post_author(p.user_id);

        if(post_author.isFound())
        {
            post_ctx["author_name"] = post_author.fullname();
            post_ctx["author_handle"] = post_author.handle();
            post_ctx["author_role"] = p.role;
            post_ctx["is_verified"] = post_author.is_verified();
            post_ctx["is_admin"] = isAdminRole(post_author.role());

            post_ctx["author_initials"] = getInitials(post_author.fullname());

            post_ctx["is_user"] =
                (p.role=="student" || p.role=="Student");

            post_ctx["is_prof"] =
                (p.role=="teacher" || p.role=="Teacher");

            post_ctx["is_staff"] =
                (p.role=="staff" || p.role=="Staff");
        }

        post_ctx["body"] = p.content;
        post_ctx["likes"] = p.likes;
        post_ctx["reposts"] = p.reposts;
        post_ctx["author_initials"] = getInitials(post_author.fullname());
        post_ctx["id"] = p.post_id;
        post_ctx["replies"] =
            reply_count.count(p.post_id) ?
            reply_count[p.post_id] : 0;

        // Add delete button flag if current user is post owner
        int currentUserID = verify_token(req);
        if (currentUserID > 0 && p.user_id == currentUserID)
            post_ctx["can_delete"] = true;

        if(p.created_at.size() >= 10)
            post_ctx["time_ago"] = p.created_at.substr(0,10);
        else
            post_ctx["time_ago"] = p.created_at;

        posts_vector.push_back(post_ctx);
    }

    ctx["posts"] = std::move(posts_vector);

    // NEWS SECTION (kept from old code)

    ctx["news"] = loadNews();

    auto page = crow::mustache::load("index.html");

    return crow::response(page.render(ctx)); });

    // ==========================================
    // 2. STUDENTS FEED ROUTE
    // ==========================================
    /*
     * STUDENTS FEED ROUTE (/students):
     * This is a special feed that only shows posts made by students.
     */
    CROW_ROUTE(app, "/students")([](const crow::request &req)
                                 {
                                    int userID = verify_token(req);
        if (userID <= 0) { crow::response res; res.code = 303; res.set_header("Location", "/login"); return res; }

        crow::mustache::context ctx; ctx["title"] = "Students | X-NCU";
        user currentUser(userID);

        if (currentUser.isFound()) {
            string initials = "U"; if (currentUser.fullname().length() >= 2) initials = currentUser.fullname().substr(0, 2);
            ctx["user_initials"] = initials; ctx["user_name"] = currentUser.fullname(); ctx["user_handle"] = currentUser.handle();
        }

        std::vector<crow::mustache::context> posts_vector;
        std::vector<crow::mustache::context> news_vector;

        std::ifstream posts_file("database/students.csv"); std::string line;
        if (posts_file.good()) std::getline(posts_file, line);

        while (std::getline(posts_file, line)) {
            if (line.empty()) continue;

            std::stringstream ss(line);
            std::string post_id, user_id, content, parent_id, likes_count, retweets_count, created_at, role;

            std::getline(ss, post_id, ','); std::getline(ss, user_id, ','); std::getline(ss, content, ',');
            std::getline(ss, parent_id, ','); std::getline(ss, likes_count, ','); std::getline(ss, retweets_count, ',');
            std::getline(ss, created_at, ','); std::getline(ss, role, ',');

            if (!role.empty() && role.back() == '\r') role.pop_back();
            if (user_id.empty()) continue;

            int safe_user_id = -1;
            try { safe_user_id = stoi(user_id); } catch (...) { continue; }

            int post_id_int = -1;
            try { post_id_int = stoi(post_id); } catch (...) { continue; }

            crow::mustache::context post_ctx;
            post_ctx["id"] = post_id_int;
            post_ctx["body"] = content; post_ctx["likes"] = likes_count; post_ctx["reposts"] = retweets_count;
            post_ctx["replies"] = 0;
            if (created_at.size() >= 10)
                post_ctx["time_ago"] = created_at.substr(0, 10);
            else
                post_ctx["time_ago"] = created_at;

            user post_author(safe_user_id);
            if(post_author.isFound()) {
                post_ctx["author_name"] = post_author.fullname();
                post_ctx["author_handle"] = post_author.handle();
                post_ctx["author_role"] = role;
                post_ctx["is_verified"] = post_author.is_verified();
                post_ctx["is_admin"] = isAdminRole(post_author.role());

                string author_initials = "";
                if (!post_author.fullname().empty()) {
                    author_initials += post_author.fullname()[0];
                    size_t space_pos = post_author.fullname().find(' ');
                    if (space_pos != string::npos && space_pos + 1 < post_author.fullname().length()) {
                        author_initials += post_author.fullname()[space_pos + 1];
                    }
                }
                if(author_initials.empty()) author_initials = "U";
                post_ctx["author_initials"] = author_initials;

                post_ctx["is_user"] = (role == "student" || role == "Student");
                post_ctx["is_prof"] = (role == "teacher" || role == "Teacher");
                post_ctx["is_staff"] = (role == "staff" || role == "Staff");
            }

            // Add delete button flag if current user is post owner
            if (userID > 0 && safe_user_id == userID)
                post_ctx["can_delete"] = true;

            posts_vector.push_back(post_ctx);
        }
        ctx["posts"] = std::move(posts_vector);

        std::ifstream news_file("database/news.csv"); if (news_file.good()) std::getline(news_file, line);
        while (std::getline(news_file, line)) {
            std::stringstream ss(line); std::string id, headline, category, time_ago, post_count;
            std::getline(ss, id, ','); std::getline(ss, headline, ','); std::getline(ss, category, ','); std::getline(ss, time_ago, ','); std::getline(ss, post_count, ',');
            crow::mustache::context news_ctx; news_ctx["headline"] = headline; news_ctx["category"] = category; news_ctx["time_ago"] = time_ago; news_ctx["post_count"] = post_count;
            news_vector.push_back(news_ctx);
        }
        ctx["news"] = std::move(news_vector);

        auto page = crow::mustache::load("index.html"); return crow::response(page.render(ctx)); });

    // ==========================================
    // 3. TEACHERS FEED ROUTE
    // ==========================================
    CROW_ROUTE(app, "/teachers")([](const crow::request &req)
                                 {
                                    int userID = verify_token(req);
        if (userID<= 0) { crow::response res; res.code = 303; res.set_header("Location", "/login"); return res; }

        crow::mustache::context ctx; ctx["title"] = "Teachers | X-NCU";
        user currentUser(userID);

        if (currentUser.isFound()) {
            string initials = "U"; if (currentUser.fullname().length() >= 2) initials = currentUser.fullname().substr(0, 2);
            ctx["user_initials"] = initials; ctx["user_name"] = currentUser.fullname(); ctx["user_handle"] = currentUser.handle();
        }

        std::vector<crow::mustache::context> posts_vector;
        std::vector<crow::mustache::context> news_vector;

        std::ifstream posts_file("database/teachers.csv"); std::string line;
        if (posts_file.good()) std::getline(posts_file, line);

        while (std::getline(posts_file, line)) {
            if (line.empty()) continue;

            std::stringstream ss(line);
            std::string post_id, user_id, content, parent_id, likes_count, retweets_count, created_at, role;

            std::getline(ss, post_id, ','); std::getline(ss, user_id, ','); std::getline(ss, content, ',');
            std::getline(ss, parent_id, ','); std::getline(ss, likes_count, ','); std::getline(ss, retweets_count, ',');
            std::getline(ss, created_at, ','); std::getline(ss, role, ',');

            if (!role.empty() && role.back() == '\r') role.pop_back();
            if (user_id.empty()) continue;

            int safe_user_id = -1;
            try { safe_user_id = stoi(user_id); } catch (...) { continue; }

            int post_id_int = -1;
            try { post_id_int = stoi(post_id); } catch (...) { continue; }

            crow::mustache::context post_ctx;
            post_ctx["id"] = post_id_int;
            post_ctx["body"] = content; post_ctx["likes"] = likes_count; post_ctx["reposts"] = retweets_count;
            post_ctx["replies"] = 0;
            if (created_at.size() >= 10)
                post_ctx["time_ago"] = created_at.substr(0, 10);
            else
                post_ctx["time_ago"] = created_at;

            user post_author(safe_user_id);
            if(post_author.isFound()) {
                post_ctx["author_name"] = post_author.fullname();
                post_ctx["author_handle"] = post_author.handle();
                post_ctx["author_role"] = role;
                post_ctx["is_verified"] = post_author.is_verified();
                post_ctx["is_admin"] = isAdminRole(post_author.role());

                string author_initials = "";
                if (!post_author.fullname().empty()) {
                    author_initials += post_author.fullname()[0];
                    size_t space_pos = post_author.fullname().find(' ');
                    if (space_pos != string::npos && space_pos + 1 < post_author.fullname().length()) {
                        author_initials += post_author.fullname()[space_pos + 1];
                    }
                }
                if(author_initials.empty()) author_initials = "U";
                post_ctx["author_initials"] = author_initials;

                post_ctx["is_user"] = (role == "student" || role == "Student");
                post_ctx["is_prof"] = (role == "teacher" || role == "Teacher");
                post_ctx["is_staff"] = (role == "staff" || role == "Staff");
            }

            // Add delete button flag if current user is post owner
            if (userID > 0 && safe_user_id == userID)
                post_ctx["can_delete"] = true;

            posts_vector.push_back(post_ctx);
        }
        ctx["posts"] = std::move(posts_vector);

        std::ifstream news_file("database/news.csv"); if (news_file.good()) std::getline(news_file, line);
        while (std::getline(news_file, line)) {
            std::stringstream ss(line); std::string id, headline, category, time_ago, post_count;
            std::getline(ss, id, ','); std::getline(ss, headline, ','); std::getline(ss, category, ','); std::getline(ss, time_ago, ','); std::getline(ss, post_count, ',');
            crow::mustache::context news_ctx; news_ctx["headline"] = headline; news_ctx["category"] = category; news_ctx["time_ago"] = time_ago; news_ctx["post_count"] = post_count;
            news_vector.push_back(news_ctx);
        }
        ctx["news"] = std::move(news_vector);

        auto page = crow::mustache::load("index.html"); return crow::response(page.render(ctx)); });

    // ==========================================
    // 4. STAFF FEED ROUTE
    // ==========================================
    CROW_ROUTE(app, "/staff")([](const crow::request &req)
                              {
        int userID = verify_token(req);
        if (userID<= 0) { crow::response res; res.code = 303; res.set_header("Location", "/login"); return res; }

        crow::mustache::context ctx; ctx["title"] = "Staff | X-NCU";
        user currentUser(userID);

        if (currentUser.isFound()) {
            string initials = "U"; if (currentUser.fullname().length() >= 2) initials = currentUser.fullname().substr(0, 2);
            ctx["user_initials"] = initials; ctx["user_name"] = currentUser.fullname(); ctx["user_handle"] = currentUser.handle();
        }

        std::vector<crow::mustache::context> posts_vector;
        std::vector<crow::mustache::context> news_vector;

        std::ifstream posts_file("database/staff.csv"); std::string line;
        if (posts_file.good()) std::getline(posts_file, line);

        while (std::getline(posts_file, line)) {
            if (line.empty()) continue;

            std::stringstream ss(line);
            std::string post_id, user_id, content, parent_id, likes_count, retweets_count, created_at, role;

            std::getline(ss, post_id, ','); std::getline(ss, user_id, ','); std::getline(ss, content, ',');
            std::getline(ss, parent_id, ','); std::getline(ss, likes_count, ','); std::getline(ss, retweets_count, ',');
            std::getline(ss, created_at, ','); std::getline(ss, role, ',');

            if (!role.empty() && role.back() == '\r') role.pop_back();
            if (user_id.empty()) continue;

            int safe_user_id = -1;
            try { safe_user_id = stoi(user_id); } catch (...) { continue; }

            int post_id_int = -1;
            try { post_id_int = stoi(post_id); } catch (...) { continue; }

            crow::mustache::context post_ctx;
            post_ctx["id"] = post_id_int;
            post_ctx["body"] = content; post_ctx["likes"] = likes_count; post_ctx["reposts"] = retweets_count;
            post_ctx["replies"] = 0;
            if (created_at.size() >= 10)
                post_ctx["time_ago"] = created_at.substr(0, 10);
            else
                post_ctx["time_ago"] = created_at;

            user post_author(safe_user_id);
            if(post_author.isFound()) {
                post_ctx["author_name"] = post_author.fullname();
                post_ctx["author_handle"] = post_author.handle();
                post_ctx["author_role"] = role;
                post_ctx["is_verified"] = post_author.is_verified();
                post_ctx["is_admin"] = isAdminRole(post_author.role());

                string author_initials = "";
                if (!post_author.fullname().empty()) {
                    author_initials += post_author.fullname()[0];
                    size_t space_pos = post_author.fullname().find(' ');
                    if (space_pos != string::npos && space_pos + 1 < post_author.fullname().length()) {
                        author_initials += post_author.fullname()[space_pos + 1];
                    }
                }
                if(author_initials.empty()) author_initials = "U";
                post_ctx["author_initials"] = author_initials;

                post_ctx["is_user"] = (role == "student" || role == "Student");
                post_ctx["is_prof"] = (role == "teacher" || role == "Teacher");
                post_ctx["is_staff"] = (role == "staff" || role == "Staff");
            }

            // Add delete button flag if current user is post owner
            if (userID > 0 && safe_user_id == userID)
                post_ctx["can_delete"] = true;

            posts_vector.push_back(post_ctx);
        }
        ctx["posts"] = std::move(posts_vector);

        std::ifstream news_file("database/news.csv"); if (news_file.good()) std::getline(news_file, line);
        while (std::getline(news_file, line)) {
            std::stringstream ss(line); std::string id, headline, category, time_ago, post_count;
            std::getline(ss, id, ','); std::getline(ss, headline, ','); std::getline(ss, category, ','); std::getline(ss, time_ago, ','); std::getline(ss, post_count, ',');
            crow::mustache::context news_ctx; news_ctx["headline"] = headline; news_ctx["category"] = category; news_ctx["time_ago"] = time_ago; news_ctx["post_count"] = post_count;
            news_vector.push_back(news_ctx);
        }
        ctx["news"] = std::move(news_vector);

        auto page = crow::mustache::load("index.html"); return crow::response(page.render(ctx)); });

    // GET ABOUT PAGE
    CROW_ROUTE(app, "/logout")([]()
                               {
                                crow::response res;
                                res.set_header("Set-Cookie", "token=; HttpOnly; Path=/; Max-Age=0");
                                res.code = 303;
                                res.set_header("Location", "/login");
                               return res; });

    CROW_ROUTE(app, "/about")([](const crow::request &req)
                              {
        auto variable_page = crow::mustache::load("about.html");
        return variable_page.render(); });

    // GET LOGIN PAGE
    CROW_ROUTE(app, "/login")([](const crow::request &req)
                              {
        if (verify_token(req) > 0) { crow::response res; res.code = 303; res.set_header("Location", "/"); return res; }
        auto variable_page = crow::mustache::load("login.html");
        return crow::response(variable_page.render()); });

    // GET SIGNUP PAGE
    CROW_ROUTE(app, "/signup")([](const crow::request &req)
                               {
        if (verify_token(req) > 0) { crow::response res; res.code = 303; res.set_header("Location", "/"); return res; }
        auto variable_page = crow::mustache::load("signup.html");
        return crow::response(variable_page.render()); });

    // POST SEND OTP
    CROW_ROUTE(app, "/send-otp").methods(crow::HTTPMethod::POST)([](const crow::request &req)
                                                                 {
        crow::query_string params("?" + req.body);
        std::string email = params.get("email") ? params.get("email") : "";
        if (email.empty()) return crow::response(400, "Email required");
        if (otp::OTPService::getInstance().generateAndSendOTP(email)) {
            return crow::response(200, "OTP Sent");
        }
        return crow::response(500, "Failed to send OTP"); });

    // POST REGISTER DATA
    CROW_ROUTE(app, "/register").methods(crow::HTTPMethod::POST)([](const crow::request &req)
                                                                 {
        crow::query_string params("?" + req.body);

        std::string name = params.get("name") ? params.get("name") : "";
        std::string email = params.get("email") ? params.get("email") : "";
        std::string role = params.get("role") ? params.get("role") : "";
        std::string password = params.get("password") ? params.get("password") : "";
        std::string otp_val = params.get("otp") ? params.get("otp") : "";

        if (name.empty() || email.empty() || role.empty() || password.empty() || otp_val.empty())
        {
            return crow::response(400, "Error: All fields are required!");
        }

        if (!otp::OTPService::getInstance().verifyOTP(email, otp_val)) {
            auto message_page = crow::mustache::load("message.html");
            crow::mustache::context ctx({{"error_code", "401"}, {"error_message", "Invalid or Expired OTP"}});
            return crow::response(401, message_page.render(ctx));
        }

        int signup_status = registerUser(email, name, role, password);
        int userId = signup_status;


        if (userId == -1)
        {
            auto message_page = crow::mustache::load("message.html");
            crow::mustache::context ctx({{"error_code", "422"}, {"error_message", "User already exists"}});
            return crow::response(422, message_page.render(ctx));
        }
        else
        {
            std::string token = jwt::create()
    .set_issuer("x-ncu")
    .set_payload_claim("user_id", jwt::claim(std::to_string(userId)))
    .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours(24))
    .sign(jwt::algorithm::hs256{"meetthemakeraditya"});

            crow::response res;
            res.code = 303;
            res.set_header("Set-Cookie", "token=" + token + "; HttpOnly; Path=/");
            res.set_header("Location", "/");
            return res;
        } });

    // GET PROFILE PAGE
    /*
     * PROFILE PAGE ROUTE (/profile/<username>):
     * This loads a specific user's personal page.
     * It shows their details and only the posts they have made.
     */
    CROW_ROUTE(app, "/profile/<string>")([](const crow::request &req, string username)
                                         {

    // requireLogin(req);
    int userID = verify_token(req);

    if (username[0] != '@')
        username = "@" + username;

    int search_userID = getuserprofile(username);

    if (search_userID <= 0)
    {
        auto message_page = crow::mustache::load("message.html");
        crow::mustache::context ctx({{"error_code", "404"}, {"error_message", "User Not Found"}});
        return crow::response(404, message_page.render(ctx));
    }

    user profileUser(search_userID);

    crow::mustache::context ctx;
    user currentUser(userID);

    if (currentUser.isFound())
    {
        string initials = "U";

        if (!currentUser.fullname().empty())
        {
            initials = "";
            initials += currentUser.fullname()[0];

            size_t space = currentUser.fullname().find(' ');
            if (space != string::npos && space + 1 < currentUser.fullname().size())
                initials += currentUser.fullname()[space + 1];
        }

        ctx["user_initials"] = initials;
        ctx["user_name"] = currentUser.fullname();
        ctx["user_handle"] = currentUser.handle();

        ctx["profile_name"] = profileUser.fullname();
        ctx["profile_handle"] = profileUser.handle();
        ctx["profile_user_id"] = profileUser.id();
        string name = profileUser.fullname();
        ctx["profile_initials"] = name.size() >= 2 ? name.substr(0,2) : name;
        ctx["profile_bio"] = profileUser.bio();
        ctx["is_verified"] = profileUser.is_verified();
        ctx["is_admin"] = isAdminRole(profileUser.role());

        ctx["profile_location"] = profileUser.location();
        ctx["profile_link"] = profileUser.link();
        ctx["profile_following"] = profileUser.following_count();
        ctx["profile_followers"] = profileUser.followers_count();
        ctx["profile_posts"] = profileUser.posts();

        if (profileUser.created_at().size() >= 10)
            ctx["profile_join_date"] = profileUser.created_at().substr(0, 10);
        else
            ctx["profile_join_date"] = profileUser.created_at();

        if (profileUser.id() == currentUser.id())
            ctx["is_own_profile"] = true;
        else if (doesUserFollow(currentUser.id(), profileUser.id()))
            ctx["is_following"] = true;

        // Load posts for this user
        std::vector<crow::mustache::context> posts_vector;
        std::ifstream posts_file("database/posts.csv");
        std::string line;

        // Pre-compute profile initials
        std::string profile_initials_str = "";
        if (!profileUser.fullname().empty()) {
            profile_initials_str += profileUser.fullname()[0];
            size_t space_pos = profileUser.fullname().find(' ');
            if (space_pos != std::string::npos && space_pos + 1 < profileUser.fullname().length()) {
                profile_initials_str += profileUser.fullname()[space_pos + 1];
            }
        }
        if (profile_initials_str.empty()) profile_initials_str = "U";

        if (posts_file.good())
            std::getline(posts_file, line); // skip header

        while (std::getline(posts_file, line))
        {
            if (line.empty()) continue;

            std::stringstream ss(line);
            std::string post_id_str, user_id_str, content, parent_id_str, likes_str, reposts_str, created_at, role;

            std::getline(ss, post_id_str, ',');
            std::getline(ss, user_id_str, ',');
            std::getline(ss, content, ',');
            std::getline(ss, parent_id_str, ',');
            std::getline(ss, likes_str, ',');
            std::getline(ss, reposts_str, ',');
            std::getline(ss, created_at, ',');
            std::getline(ss, role);

            // Remove carriage return if present
            if (!role.empty() && role.back() == '\r')
                role.pop_back();

            if (user_id_str.empty()) continue;

            int safe_user_id = -1;
            try { safe_user_id = stoi(user_id_str); } catch (...) { continue; }

            // Only load posts from the profile user
            if (safe_user_id != profileUser.id()) continue;

            int post_id = -1;
            try { post_id = stoi(post_id_str); } catch (...) { continue; }

            crow::mustache::context post_ctx;
            post_ctx["id"] = post_id;
            post_ctx["body"] = content;
            post_ctx["likes"] = likes_str;
            post_ctx["reposts"] = reposts_str;
            post_ctx["replies"] = 0;
            post_ctx["author_name"] = profileUser.fullname();
            post_ctx["author_handle"] = profileUser.handle();
            post_ctx["author_role"] = role;
            post_ctx["is_verified"] = profileUser.is_verified();
            post_ctx["is_admin"] = isAdminRole(profileUser.role());
            post_ctx["author_initials"] = profile_initials_str;

            post_ctx["is_user"] = (role == "student" || role == "Student");
            post_ctx["is_prof"] = (role == "teacher" || role == "Teacher");
            post_ctx["is_staff"] = (role == "staff" || role == "Staff");

            if (created_at.size() >= 10)
                post_ctx["time_ago"] = created_at.substr(0, 10);
            else
                post_ctx["time_ago"] = created_at;

            // Add flag for delete button (only show if current user owns post)
            if (profileUser.id() == currentUser.id())
                post_ctx["can_delete"] = true;

            posts_vector.push_back(post_ctx);
        }

        posts_file.close();

        if (!posts_vector.empty())
        {
            ctx["has_posts"] = true;
            ctx["posts"] = std::move(posts_vector);
        }
        else
        {
            ctx["has_posts"] = false;
        }

        ctx["news"] = loadNews();

        auto profile_page = crow::mustache::load("profile.html");
        return crow::response(profile_page.render(ctx));
    }

    return crow::response(404); });

    // POST AUTHENTICATION DATA
    CROW_ROUTE(app, "/auth").methods(crow::HTTPMethod::POST)([](const crow::request &req)
                                                             {
        crow::query_string params("?" + req.body);
        std::string email = params.get("email") ? params.get("email") : "";
        std::string password = params.get("password") ? params.get("password") : "";

        int userId = authenticateUser(email, password);

        if (userId > 0)
        {
            std::string token = jwt::create()
        .set_issuer("x-ncu")
        .set_payload_claim("user_id", jwt::claim(std::to_string(userId)))
        .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours(24))
        .sign(jwt::algorithm::hs256{"meetthemakeraditya"});

            crow::response res;
            res.code = 303;
            res.set_header("Set-Cookie", "token=" + token + "; HttpOnly; Path=/");
            res.set_header("Location", "/");
            return res;
        }
        else if (userId == -2)
        {
            auto message_page = crow::mustache::load("message.html");
            crow::mustache::context ctx({{"error_code", "500"}, {"error_message", "Could not open database\nSystem Error"}});
            return crow::response(500, message_page.render(ctx));
        }
        else if (userId == 0)
        {
            auto message_page = crow::mustache::load("message.html");
            crow::mustache::context ctx({{"error_code", "404"}, {"error_message", "User Not Found"}});
            return crow::response(404, message_page.render(ctx));
        }
        else
        {
            auto message_page = crow::mustache::load("message.html");
            crow::mustache::context ctx({{"error_code", "401"}, {"error_message", "Wrong password"}});
            return crow::response(401, message_page.render(ctx));
        } });

    // ==========================================
    // new post route
    // ==========================================
    CROW_ROUTE(app, "/post").methods(crow::HTTPMethod::POST)([](const crow::request &req)
                                                             {
        if (verify_token(req)<= 0) {
            crow::response res;
            res.code = 303;
            res.set_header("Location", "/login");
            return res;
        }

        crow::query_string params("?" + req.body);
        std::string content = params.get("content") ? params.get("content") : "";
        std::string parent_id_str = params.get("parent_id") ? params.get("parent_id") : "-1";

        int parent_id = -1;
        try {
            parent_id = std::stoi(parent_id_str);
        } catch (...) {
            parent_id = -1; // Default to -1 if parsing fails
        }

        if (content.empty()) {
            auto message_page = crow::mustache::load("message.html");
            crow::mustache::context ctx({{"error_code", "400"}, {"error_message", "Content cannot be empty"}});
            return crow::response(400, message_page.render(ctx));
        }

        post new_post(content, verify_token(req), parent_id);

        crow::response res;
        res.code = 303;
        res.set_header("Location", "/");
        return res; });

    CROW_ROUTE(app, "/updatepost")
        .methods(crow::HTTPMethod::GET, crow::HTTPMethod::POST)([](const crow::request &req)
                                                                {
            if (verify_token(req)<= 0)
            {
                crow::response res;
                res.code = 303;
                res.set_header("Location", "/login");
                return res;
            }

            std::string action_str = "";
            std::string id_str = "";

            if (req.method == crow::HTTPMethod::GET)
            {
                action_str = req.url_params.get("action") ? req.url_params.get("action") : "";
                id_str = req.url_params.get("id") ? req.url_params.get("id") : "";
            }
            else if (req.method == crow::HTTPMethod::POST)
            {
                crow::query_string params("?" + req.body);
                action_str = params.get("action") ? params.get("action") : "";
                id_str = params.get("id") ? params.get("id") : "";
            }

            cout << action_str << " " << id_str << endl;

            int action = -1;
            int post_id = -1;

            try { action = stoi(action_str); } catch (...) {}
            try { post_id = stoi(id_str); } catch (...) {}

            if (post_id <= 0 || action < 1 || action > 3)
            {
                auto message_page = crow::mustache::load("message.html");
                crow::mustache::context ctx({{"error_code","400"},{"error_message","Invalid post_id or action"}});
                return crow::response(400, message_page.render(ctx));
            }

            post p = post::getpost(post_id);

            if (!p.isFound())
                return crow::response(404, "Post not found");

            if (action == 1)
            {
                p.likes_count(p.likes_count() + 1);
                p.savepost(p);
            }
            else if (action == 2)
            {
                p.retweets_count(p.retweets_count() + 1);
                p.savepost(p);
            }

            crow::response res;
            res.code = 303;
            res.set_header("Location", "/");
            return res; });

    // ==========================================
    // DELETE POST ROUTE
    // ==========================================
    CROW_ROUTE(app, "/deletepost").methods(crow::HTTPMethod::GET, crow::HTTPMethod::POST)([](const crow::request &req)
                                                                                           {
        if (verify_token(req)<= 0)
        {
            crow::response res;
            res.code = 303;
            res.set_header("Location", "/login");
            return res;
        }

        std::string post_id_str = "";

        if (req.method == crow::HTTPMethod::GET)
        {
            post_id_str = req.url_params.get("id") ? req.url_params.get("id") : "";
        }
        else if (req.method == crow::HTTPMethod::POST)
        {
            crow::query_string params("?" + req.body);
            post_id_str = params.get("id") ? params.get("id") : "";
        }

        if (post_id_str.empty())
        {
            auto message_page = crow::mustache::load("message.html");
            crow::mustache::context ctx({{"error_code", "400"}, {"error_message", "Post ID is required"}});
            return crow::response(400, message_page.render(ctx));
        }

        int post_id = -1;
        try
        {
            post_id = stoi(post_id_str);
        }
        catch (...)
        {
            auto message_page = crow::mustache::load("message.html");
            crow::mustache::context ctx({{"error_code", "400"}, {"error_message", "Invalid post ID"}});
            return crow::response(400, message_page.render(ctx));
        }

        post p = post::getpost(post_id);

        if (!p.isFound())
        {
            auto message_page = crow::mustache::load("message.html");
            crow::mustache::context ctx({{"error_code", "404"}, {"error_message", "Post not found"}});
            return crow::response(404, message_page.render(ctx));
        }

        int logged_in_user = verify_token(req);

        // Check if the logged-in user is the owner of the post
        if (p.user_id() != logged_in_user)
        {
            auto message_page = crow::mustache::load("message.html");
            crow::mustache::context ctx({{"error_code", "403"}, {"error_message", "You can only delete your own posts"}});
            return crow::response(403, message_page.render(ctx));
        }

        // Delete the post
        if (post::deletepost(post_id))
        {
            crow::response res;
            res.code = 303;
            res.set_header("Location", "/");
            return res;
        }
        else
        {
            auto message_page = crow::mustache::load("message.html");
            crow::mustache::context ctx({{"error_code", "500"}, {"error_message", "Failed to delete post"}});
            return crow::response(500, message_page.render(ctx));
        } });

    CROW_ROUTE(app, "/editprofile").methods(crow::HTTPMethod::GET, crow::HTTPMethod::POST)([](const crow::request &req)
                                                                                           {
        if (verify_token(req)<= 0)
        {
            return requireLogin(req);
        }

        user currentUser(verify_token(req));

        if (!currentUser.isFound())
        {
            return crow::response(404, "User not found");
        }

        if (req.method == crow::HTTPMethod::GET)
        {
            crow::mustache::context ctx;
            string initials = "U";

            initials = getInitials(currentUser.fullname());

            addCurrentUserContext(ctx, verify_token(req));

            ctx["profile_name"] = currentUser.fullname();
            ctx["profile_handle"] = currentUser.handle();
            ctx["profile_bio"] = currentUser.bio();
            ctx["profile_location"] = currentUser.location();
            ctx["profile_link"] = currentUser.link();
            ctx["profile_initials"] = initials;
            string name = currentUser.fullname();
            auto profile_page = crow::mustache::load("edit_profile.html");
            return crow::response(profile_page.render(ctx));


        } return crow::response(200, "Profile updated successfully"); });

    CROW_ROUTE(app, "/updateprofile").methods(crow::HTTPMethod::POST)([](const crow::request &req)
                                                                                             {
        if (verify_token(req)<= 0)
        {
            return requireLogin(req);
        }

        user currentUser(verify_token(req));

        if (!currentUser.isFound())
        {
            return crow::response(404, "User not found");
        }

        crow::query_string params("?" + req.body);

        std::string fullname = params.get("fullname") ? params.get("fullname") : "";
        std::string bio = params.get("bio") ? params.get("bio") : "";
        std::string location = params.get("location") ? params.get("location") : "";
        std::string link = params.get("link") ? params.get("link") : "";

        currentUser.fullname(fullname);
        currentUser.bio(bio);
        currentUser.location(location);
        currentUser.link(link);
        currentUser.updateUserInCSV();

        if (currentUser.fullname()==fullname && currentUser.bio()==bio && currentUser.location()==location && currentUser.link()==link)
        {
            crow::response res;
            res.code = 303;
            res.set_header("Location", "/profile/" + currentUser.handle());
            return res;
        }
        else
        {
            auto message_page = crow::mustache::load("message.html");
            crow::mustache::context ctx({{"error_code", "500"}, {"error_message", "Failed to update profile"}});
            return crow::response(500, message_page.render(ctx));
        } });

    // LIKE ROUTE
    CROW_ROUTE(app, "/like").methods(crow::HTTPMethod::POST)([](const crow::request &req)
    {
        int user_id = verify_token(req);
        if (user_id <= 0) return crow::response(401, "Unauthorized");

        auto post_id = getRequestParam(req, "post_id");
        if (post_id.empty()) return crow::response(400, "Missing post_id");

        try
        {
            int postId = stoi(post_id);
            post p = post::getpost(postId);
            if (!p.isFound()) return crow::response(404, "Post not found");
            if (p.user_id() == user_id) return crow::response(400, "Cannot like own post");

            if (hasUserLikedPost(user_id, postId))
                return crow::response(400, "Already liked");

            addLikeToUser(user_id, postId);
            p.likes_count(p.likes_count() + 1);
            post::savepost(p);

            return crow::response(200, "Liked");
        }
        catch (...) { return crow::response(400, "Invalid post_id"); }
    });

    // UNLIKE ROUTE
    CROW_ROUTE(app, "/unlike").methods(crow::HTTPMethod::POST)([](const crow::request &req)
    {
        int user_id = verify_token(req);
        if (user_id <= 0) return crow::response(401, "Unauthorized");

        auto post_id = getRequestParam(req, "post_id");
        if (post_id.empty()) return crow::response(400, "Missing post_id");

        try
        {
            int postId = stoi(post_id);
            if (!hasUserLikedPost(user_id, postId))
                return crow::response(400, "Not liked");

            removeLikeFromUser(user_id, postId);
            post p = post::getpost(postId);
            if (p.isFound())
            {
                p.likes_count(max(0, p.likes_count() - 1));
                post::savepost(p);
            }

            return crow::response(200, "Unliked");
        }
        catch (...) { return crow::response(400, "Invalid post_id"); }
    });

    // FOLLOW ROUTE
    CROW_ROUTE(app, "/follow").methods(crow::HTTPMethod::POST)([](const crow::request &req)
    {
        int user_id = verify_token(req);
        if (user_id <= 0) return crow::response(401, "Unauthorized");

        auto follow_id = getRequestParam(req, "user_id");
        if (follow_id.empty()) return crow::response(400, "Missing user_id");

        try
        {
            int followId = stoi(follow_id);
            if (user_id == followId) return crow::response(400, "Cannot follow yourself");

            user targetUser(followId);
            if (!targetUser.isFound()) return crow::response(404, "User not found");

            if (doesUserFollow(user_id, followId))
                return crow::response(400, "Already following");

            addFollow(user_id, followId);
            return crow::response(200, "Followed");
        }
        catch (...) { return crow::response(400, "Invalid user_id"); }
    });

    // UNFOLLOW ROUTE
    CROW_ROUTE(app, "/unfollow").methods(crow::HTTPMethod::POST)([](const crow::request &req)
    {
        int user_id = verify_token(req);
        if (user_id <= 0) return crow::response(401, "Unauthorized");

        auto unfollow_id = getRequestParam(req, "user_id");
        if (unfollow_id.empty()) return crow::response(400, "Missing user_id");

        try
        {
            int unfollowId = stoi(unfollow_id);
            if (!doesUserFollow(user_id, unfollowId))
                return crow::response(400, "Not following");

            removeFollow(user_id, unfollowId);
            return crow::response(200, "Unfollowed");
        }
        catch (...) { return crow::response(400, "Invalid user_id"); }
    });

    // FOLLOWING LIST ROUTE
    CROW_ROUTE(app, "/following/<string>")([](const crow::request &req, string username)
    {
        int user_id = verify_token(req);
        if (user_id <= 0) return crow::response(401, "Unauthorized");

        int targetUserId = getuserprofile(username);
        if (targetUserId <= 0) return crow::response(404, "User not found");

        user targetUser(targetUserId);
        if (!targetUser.isFound()) return crow::response(404, "User not found");

        vector<int> followed = getFollowedUsers(targetUserId);

        crow::mustache::context ctx;
        ctx["target_user"] = targetUser.fullname();
        ctx["target_handle"] = targetUser.handle();

        vector<crow::mustache::context> following_list;
        for (int followedId : followed)
        {
            user u(followedId);
            if (u.isFound())
            {
                crow::mustache::context user_ctx;
                user_ctx["id"] = u.id();
                user_ctx["fullname"] = u.fullname();
                user_ctx["handle"] = u.handle();
                user_ctx["bio"] = u.bio();
                following_list.push_back(user_ctx);
            }
        }
        ctx["following"] = std::move(following_list);
        ctx["news"] = loadNews();
        addCurrentUserContext(ctx, user_id);

        auto page = crow::mustache::load("following.html");
        return crow::response(page.render(ctx));
    });

    // FOLLOWERS LIST ROUTE
    CROW_ROUTE(app, "/followers/<string>")([](const crow::request &req, string username)
    {
        int user_id = verify_token(req);
        if (user_id <= 0) return crow::response(401, "Unauthorized");

        int targetUserId = getuserprofile(username);
        if (targetUserId <= 0) return crow::response(404, "User not found");

        user targetUser(targetUserId);
        if (!targetUser.isFound()) return crow::response(404, "User not found");

        vector<crow::mustache::context> followers_list;
        ifstream file("database/users.csv");
        if (file.is_open())
        {
            string line;
            getline(file, line);
            while (getline(file, line))
            {
                if (line.empty()) continue;
                stringstream ss(line);
                string idStr;
                getline(ss, idStr, ',');
                try
                {
                    int checkId = stoi(idStr);
                    if (doesUserFollow(checkId, targetUserId))
                    {
                        user follower(checkId);
                        if (follower.isFound())
                        {
                            crow::mustache::context user_ctx;
                            user_ctx["id"] = follower.id();
                            user_ctx["fullname"] = follower.fullname();
                            user_ctx["handle"] = follower.handle();
                            user_ctx["bio"] = follower.bio();
                            followers_list.push_back(user_ctx);
                        }
                    }
                }
                catch (...) {}
            }
            file.close();
        }

        crow::mustache::context ctx;
        ctx["target_user"] = targetUser.fullname();
        ctx["target_handle"] = targetUser.handle();
        ctx["followers"] = std::move(followers_list);
        ctx["news"] = loadNews();
        addCurrentUserContext(ctx, user_id);

        auto page = crow::mustache::load("followers.html");
        return crow::response(page.render(ctx));
    });

    // ACADEMIC HUBS INDEX
    CROW_ROUTE(app, "/hubs")([](const crow::request &req)
    {
        int user_id = verify_token(req);
        if (user_id <= 0) return requireLogin(req);

        user currentUser(user_id);
        if (!currentUser.isFound()) return crow::response(404, "User not found");

        crow::mustache::context ctx;
        ctx["title"] = "Academic Hubs | X-NCU";
        ctx["current_programme"] = currentUser.programme();
        ctx["hubs"] = buildHubContexts(currentUser.programme());
        ctx["news"] = loadNews();
        addCurrentUserContext(ctx, user_id);

        auto page = crow::mustache::load("hubs.html");
        return crow::response(page.render(ctx));
    });

    CROW_ROUTE(app, "/cse")([](const crow::request &req)
    {
        if (verify_token(req) <= 0) return requireLogin(req);
        crow::response res;
        res.code = 303;
        res.set_header("Location", "/hub/computer-applications");
        return res;
    });

    CROW_ROUTE(app, "/ece")([](const crow::request &req)
    {
        if (verify_token(req) <= 0) return requireLogin(req);
        crow::response res;
        res.code = 303;
        res.set_header("Location", "/hub/engineering-technology");
        return res;
    });

    CROW_ROUTE(app, "/me")([](const crow::request &req)
    {
        if (verify_token(req) <= 0) return requireLogin(req);
        crow::response res;
        res.code = 303;
        res.set_header("Location", "/hub/engineering-technology");
        return res;
    });

    // ACADEMIC HUB ROUTE
    CROW_ROUTE(app, "/hub/<string>")([](const crow::request &req, string hubSlug)
    {
        int user_id = verify_token(req);
        if (user_id <= 0) return requireLogin(req);

        user currentUser(user_id);
        if (!currentUser.isFound()) return crow::response(404, "User not found");

        string programme = hubProgrammeFromSlug(hubSlug);
        if (programme.empty()) return crow::response(404, "Hub not found");

        bool hasAccess = isAdminRole(currentUser.role()) || currentUser.programme() == programme;

        ifstream approvedFile("database/approved_access.csv");
        if (approvedFile.is_open())
        {
            string line;
            getline(approvedFile, line);
            while (getline(approvedFile, line))
            {
                if (line.empty()) continue;
                stringstream ss(line);
                string userIdStr;
                getline(ss, userIdStr, ',');
                try
                {
                    if (stoi(userIdStr) == user_id)
                    {
                        string allowedProgrammes;
                        getline(ss, allowedProgrammes, ',');
                        if (allowedProgrammes.find(programme) != string::npos)
                            hasAccess = true;
                        break;
                    }
                }
                catch (...) {}
            }
            approvedFile.close();
        }

        vector<crow::mustache::context> posts_list;
        if (hasAccess)
        {
            ifstream postsFile("database/posts.csv");
            if (postsFile.is_open())
            {
                string line;
                getline(postsFile, line);
                while (getline(postsFile, line))
                {
                    if (line.empty()) continue;
                    stringstream ss(line);
                    string postIdStr, userIdStr, content, parentId, likes, reposts, createdAt, role;
                    getline(ss, postIdStr, ',');
                    getline(ss, userIdStr, ',');
                    getline(ss, content, ',');
                    getline(ss, parentId, ',');
                    getline(ss, likes, ',');
                    getline(ss, reposts, ',');
                    getline(ss, createdAt, ',');
                    getline(ss, role);

                    try
                    {
                        int authorId = stoi(userIdStr);
                        user author(authorId);
                        if (author.isFound() && author.programme() == programme)
                        {
                            crow::mustache::context post_ctx;
                            post_ctx["post_id"] = postIdStr;
                            post_ctx["author"] = author.fullname();
                            post_ctx["content"] = content;
                            post_ctx["author_handle"] = author.handle();
                            post_ctx["author_initials"] = getInitials(author.fullname());
                            post_ctx["is_admin"] = isAdminRole(author.role());
                            post_ctx["is_verified"] = author.is_verified();
                            post_ctx["likes"] = likes;
                            post_ctx["time_ago"] = createdAt.size() >= 10 ? createdAt.substr(0, 10) : createdAt;
                            post_ctx["liked"] = hasUserLikedPost(user_id, stoi(postIdStr));
                            posts_list.push_back(post_ctx);
                        }
                    }
                    catch (...) {}
                }
                postsFile.close();
            }
        }

        crow::mustache::context ctx;
        ctx["programme"] = programme;
        ctx["hub_slug"] = hubSlugFromProgramme(programme);
        ctx["has_access"] = hasAccess;
        if (!hasAccess)
            ctx["needs_access"] = true;
        ctx["posts"] = std::move(posts_list);
        ctx["news"] = loadNews();
        addCurrentUserContext(ctx, user_id);

        auto page = crow::mustache::load("hub.html");
        return crow::response(page.render(ctx));
    });

    // REQUEST ACCESS ROUTE
    CROW_ROUTE(app, "/request-access").methods(crow::HTTPMethod::POST)([](const crow::request &req)
    {
        int user_id = verify_token(req);
        if (user_id <= 0) return crow::response(401, "Unauthorized");

        auto target_programme = getRequestParam(req, "programme");
        if (target_programme.empty()) return crow::response(400, "Missing programme");
        string mappedProgramme = hubProgrammeFromSlug(target_programme);
        if (!mappedProgramme.empty())
            target_programme = mappedProgramme;

        auto now = chrono::system_clock::now();
        auto time_t = chrono::system_clock::to_time_t(now);
        stringstream ss;
        ss << put_time(gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");

        ifstream file("database/access_requests.csv");
        int maxId = 0;
        if (file.is_open())
        {
            string line;
            getline(file, line);
            while (getline(file, line))
            {
                if (line.empty()) continue;
                stringstream lss(line);
                string idStr;
                getline(lss, idStr, ',');
                try { maxId = max(maxId, stoi(idStr)); }
                catch (...) {}
            }
            file.close();
        }

        int newId = maxId + 1;
        ofstream outFile("database/access_requests.csv", ios::app);
        outFile << newId << "," << user_id << "," << target_programme << ",PENDING," << ss.str() << ",,\n";
        outFile.close();

        return crow::response(200, "Request submitted");
    });

    // PENDING REQUESTS ROUTE (Admin Only)
    CROW_ROUTE(app, "/pending-requests")([](const crow::request &req)
    {
        int user_id = verify_token(req);
        if (user_id <= 0) return crow::response(401, "Unauthorized");

        user currentUser(user_id);
        if (!currentUser.isFound() || !isStaffRole(currentUser.role()))
            return crow::response(403, "Admin only");

        vector<crow::mustache::context> requests_list;
        ifstream file("database/access_requests.csv");
        if (file.is_open())
        {
            string line;
            getline(file, line);
            while (getline(file, line))
            {
                if (line.empty()) continue;
                stringstream ss(line);
                string requestId, userId, targetProg, status;
                getline(ss, requestId, ',');
                getline(ss, userId, ',');
                getline(ss, targetProg, ',');
                getline(ss, status, ',');

                if (status == "PENDING")
                {
                    user requester(stoi(userId));
                    crow::mustache::context req_ctx;
                    req_ctx["request_id"] = requestId;
                    req_ctx["user_id"] = userId;
                    req_ctx["user_name"] = requester.isFound() ? requester.fullname() : "Unknown";
                    req_ctx["target_programme"] = targetProg;
                    requests_list.push_back(req_ctx);
                }
            }
            file.close();
        }

        crow::mustache::context ctx;
        ctx["requests"] = std::move(requests_list);
        ctx["news"] = loadNews();
        addCurrentUserContext(ctx, user_id);

        auto page = crow::mustache::load("admin_requests.html");
        return crow::response(page.render(ctx));
    });

    // APPROVE REQUEST ROUTE (Admin Only)
    CROW_ROUTE(app, "/approve-request").methods(crow::HTTPMethod::POST)([](const crow::request &req)
    {
        int user_id = verify_token(req);
        if (user_id <= 0) return crow::response(401, "Unauthorized");

        user currentUser(user_id);
        if (!currentUser.isFound() || !isStaffRole(currentUser.role()))
            return crow::response(403, "Admin only");

        auto request_id = getRequestParam(req, "request_id");
        if (request_id.empty()) return crow::response(400, "Missing request_id");

        vector<string> lines;
        string targetUserId, targetProgramme;
        ifstream inFile("database/access_requests.csv");
        if (inFile.is_open())
        {
            string line;
            getline(inFile, line);
            lines.push_back(line);

            auto now = chrono::system_clock::now();
            auto time_t = chrono::system_clock::to_time_t(now);
            stringstream ss;
            ss << put_time(gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");

            while (getline(inFile, line))
            {
                if (line.empty()) continue;
                stringstream lss(line);
                string reqId;
                getline(lss, reqId, ',');

                if (reqId == request_id)
                {
                    string userId, prog, status;
                    getline(lss, userId, ',');
                    getline(lss, prog, ',');
                    getline(lss, status, ',');
                    targetUserId = userId;
                    targetProgramme = prog;

                    stringstream updated;
                    updated << reqId << "," << userId << "," << prog << ",APPROVED," << ss.str() << "," << user_id;
                    lines.push_back(updated.str());
                }
                else
                {
                    lines.push_back(line);
                }
            }
            inFile.close();
        }

        ofstream outFile("database/access_requests.csv");
        for (const auto &l : lines)
            outFile << l << "\n";
        outFile.close();

        if (!targetUserId.empty())
        {
            ifstream approvedIn("database/approved_access.csv");
            vector<string> approvedLines;
            if (approvedIn.is_open())
            {
                string line;
                getline(approvedIn, line);
                approvedLines.push_back(line);

                bool found = false;
                while (getline(approvedIn, line))
                {
                    if (line.empty()) continue;
                    stringstream ss(line);
                    string uId;
                    getline(ss, uId, ',');

                    if (uId == targetUserId)
                    {
                        string allowedProgs;
                        getline(ss, allowedProgs, ',');
                        if (allowedProgs.find(targetProgramme) == string::npos)
                        {
                            if (!allowedProgs.empty()) allowedProgs += ";";
                            allowedProgs += targetProgramme;
                        }

                        auto now = chrono::system_clock::now();
                        auto time_t = chrono::system_clock::to_time_t(now);
                        stringstream tss;
                        tss << put_time(gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");

                        stringstream updated;
                        updated << uId << "," << allowedProgs << "," << tss.str() << "," << user_id;
                        approvedLines.push_back(updated.str());
                        found = true;
                    }
                    else
                    {
                        approvedLines.push_back(line);
                    }
                }

                if (!found)
                {
                    auto now = chrono::system_clock::now();
                    auto time_t = chrono::system_clock::to_time_t(now);
                    stringstream tss;
                    tss << put_time(gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");

                    stringstream newRecord;
                    newRecord << targetUserId << "," << targetProgramme << "," << tss.str() << "," << user_id;
                    approvedLines.push_back(newRecord.str());
                }

                approvedIn.close();
            }

            ofstream approvedOut("database/approved_access.csv");
            for (const auto &l : approvedLines)
                approvedOut << l << "\n";
            approvedOut.close();
        }

        return crow::response(200, "Request approved");
    });

    // REJECT REQUEST ROUTE (Admin Only)
    CROW_ROUTE(app, "/reject-request").methods(crow::HTTPMethod::POST)([](const crow::request &req)
    {
        int user_id = verify_token(req);
        if (user_id <= 0) return crow::response(401, "Unauthorized");

        user currentUser(user_id);
        if (!currentUser.isFound() || !isStaffRole(currentUser.role()))
            return crow::response(403, "Admin only");

        auto request_id = getRequestParam(req, "request_id");
        if (request_id.empty()) return crow::response(400, "Missing request_id");

        vector<string> lines;
        ifstream inFile("database/access_requests.csv");
        if (inFile.is_open())
        {
            string line;
            getline(inFile, line);
            lines.push_back(line);

            auto now = chrono::system_clock::now();
            auto time_t = chrono::system_clock::to_time_t(now);
            stringstream ss;
            ss << put_time(gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");

            while (getline(inFile, line))
            {
                if (line.empty()) continue;
                stringstream lss(line);
                string reqId;
                getline(lss, reqId, ',');

                if (reqId == request_id)
                {
                    string userId, prog;
                    getline(lss, userId, ',');
                    getline(lss, prog, ',');

                    stringstream updated;
                    updated << reqId << "," << userId << "," << prog << ",REJECTED," << ss.str() << "," << user_id;
                    lines.push_back(updated.str());
                }
                else
                {
                    lines.push_back(line);
                }
            }
            inFile.close();
        }

        ofstream outFile("database/access_requests.csv");
        for (const auto &l : lines)
            outFile << l << "\n";
        outFile.close();

        return crow::response(200, "Request rejected");
    });

    app.bindaddr("127.0.0.1").port(18080).run();
}

#include "crow.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>
#include <unordered_map>
#include <algorithm>

using namespace std;

int global_login_stats = -1;

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
};

user::user() {}

// Updated constructor to populate the private variables
user::user(int targetUserNo)
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

        stringstream ss(line);
        string idStr;

        getline(ss, idStr, ',');

        if (idStr.empty())
            continue; // Skip if no ID is found

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
            getline(ss, posts_str);
            posts_private = posts_str.empty() ? 0 : stoi(posts_str);

            isFound_private = true;
            break;
        }
    }
}

user::~user() {}

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
        string idStr, email, fullName, handle, role, storedPassword, is_verified, bio, created_at;
        string location, link, following, followers, posts;

        // 4. Extract each column up to the comma
        getline(ss, idStr, ',');
        getline(ss, handle, ',');
        getline(ss, email, ',');
        getline(ss, fullName, ',');
        getline(ss, role, ',');
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
            if (storedPassword == inputPassword)
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
            << role << ","
            << password << ","
            << "" << ","        // Bio
            << "FALSE" << ","   // IsVerified
            << oss.str() << "," // CreatedAt
            << "" << ","        // Location
            << "" << ","        // Link
            << "0" << ","       // FollowingCount
            << "0" << ","       // FollowersCount
            << "" << "\n";      // Posts

    outFile.close();

    return newId;
}

user Current_User(global_login_stats);

class post
{
private:
    // PostID,UserID,Content,ParentID,LikesCount,RetweetsCount,CreatedAt,role
    int id_private;
    int user_id_private = global_login_stats;
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
    post(const string &content, int parent_id = -1);
    ~post();
    static post getpost(int id);
    static void savepost(const post &p);
};

post::post()
{
}

post::~post()
{
}

post::post(const string &content_input, int parent_id)
{
    id_private = 0;
    user_id_private = global_login_stats;
    parent_id_private = parent_id;
    likes_count_private = 0;
    retweets_count_private = 0;
    created_at_private = "";
    role_private = "";
    isFound_private = false;

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

    user author(global_login_stats);
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

int main()
{
    crow::SimpleApp app;

    crow::mustache::set_base(".");

    // ==========================================
    // 1. HOME ROUTE
    // ==========================================
    CROW_ROUTE(app, "/")([]()
                         {
    if (global_login_stats <= 0)
    {
        crow::response res;
        res.code = 303;
        res.set_header("Location", "/login");
        return res;
    }

    crow::mustache::context ctx;
    ctx["title"] = "HOME | X-NCU";

    user currentUser(global_login_stats);

    if (currentUser.isFound())
    {
        std::string initials = "U";
        if (currentUser.fullname().length() >= 2)
            initials = currentUser.fullname().substr(0,2);

        ctx["user_initials"] = initials;
        ctx["user_name"] = currentUser.fullname();
        ctx["user_handle"] = currentUser.handle();
    }

    std::vector<crow::mustache::context> posts_vector;
    std::vector<crow::mustache::context> news_vector;

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

            std::string initials;

            if(!post_author.fullname().empty())
            {
                initials += post_author.fullname()[0];

                size_t space = post_author.fullname().find(' ');
                if(space != std::string::npos && space+1 < post_author.fullname().size())
                    initials += post_author.fullname()[space+1];
            }

            if(initials.empty()) initials = "U";

            post_ctx["author_initials"] = initials;

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
        post_ctx["id"] = p.post_id;
        post_ctx["replies"] =
            reply_count.count(p.post_id) ?
            reply_count[p.post_id] : 0;

        if(p.created_at.size() >= 10)
            post_ctx["time_ago"] = p.created_at.substr(0,10);
        else
            post_ctx["time_ago"] = p.created_at;

        posts_vector.push_back(post_ctx);
    }

    ctx["posts"] = std::move(posts_vector);

    // NEWS SECTION (kept from old code)

    std::ifstream news_file("database/news.csv");

    if(news_file.good())
        std::getline(news_file,line);

    while(std::getline(news_file,line))
    {
        std::stringstream ss(line);

        std::string id;
        std::string headline;
        std::string category;
        std::string time_ago;
        std::string post_count;

        std::getline(ss,id,',');
        std::getline(ss,headline,',');
        std::getline(ss,category,',');
        std::getline(ss,time_ago,',');
        std::getline(ss,post_count,',');

        crow::mustache::context news_ctx;

        news_ctx["headline"] = headline;
        news_ctx["category"] = category;
        news_ctx["time_ago"] = time_ago;
        news_ctx["post_count"] = post_count;
        

        news_vector.push_back(news_ctx);
    }

    ctx["news"] = std::move(news_vector);

    auto page = crow::mustache::load("index.html");

    return crow::response(page.render(ctx)); });

    // ==========================================
    // 2. STUDENTS FEED ROUTE
    // ==========================================
    CROW_ROUTE(app, "/students")([]()
                                 {
        if (global_login_stats <= 0) { crow::response res; res.code = 303; res.set_header("Location", "/login"); return res; }

        crow::mustache::context ctx; ctx["title"] = "Students | X-NCU";
        user currentUser(global_login_stats);
        
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
        
            crow::mustache::context post_ctx;
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
    CROW_ROUTE(app, "/teachers")([]()
                                 {
        if (global_login_stats <= 0) { crow::response res; res.code = 303; res.set_header("Location", "/login"); return res; }

        crow::mustache::context ctx; ctx["title"] = "Teachers | X-NCU";
        user currentUser(global_login_stats);
        
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
        
            crow::mustache::context post_ctx;
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
    CROW_ROUTE(app, "/staff")([]()
                              {
        if (global_login_stats <= 0) { crow::response res; res.code = 303; res.set_header("Location", "/login"); return res; }

        crow::mustache::context ctx; ctx["title"] = "Staff | X-NCU";
        user currentUser(global_login_stats);
        
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
        
            crow::mustache::context post_ctx;
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
                               {global_login_stats = -1;
                               crow::response res;
                               res.code = 303;
                               res.set_header("Location", "/");
                               return res; });

    CROW_ROUTE(app, "/about")([]()
                              {
        auto variable_page = crow::mustache::load("about.html");
        return variable_page.render(); });

    // GET LOGIN PAGE
    CROW_ROUTE(app, "/login")([]()
                              {
        auto variable_page = crow::mustache::load("login.html");
        return variable_page.render(); });

    // GET SIGNUP PAGE
    CROW_ROUTE(app, "/signup")([]()
                               {
        auto variable_page = crow::mustache::load("signup.html");
        return variable_page.render(); });

    // POST REGISTER DATA
    CROW_ROUTE(app, "/register").methods(crow::HTTPMethod::POST)([](const crow::request &req)
                                                                 {
        crow::query_string params("?" + req.body);

        std::string name = params.get("name") ? params.get("name") : "";
        std::string email = params.get("email") ? params.get("email") : "";
        std::string role = params.get("role") ? params.get("role") : "";
        std::string password = params.get("password") ? params.get("password") : "";

        if (name.empty() || email.empty() || role.empty() || password.empty())
        {
            return crow::response(400, "Error: All fields are required!");
        }

        int signup_status = registerUser(email, name, role, password);
        global_login_stats = signup_status;

        if (signup_status == -1)
        {
            auto message_page = crow::mustache::load("message.html");
            crow::mustache::context ctx({{"error_code", "422"}, {"error_message", "User already exists"}});
            return crow::response(422, message_page.render(ctx));
        }
        else
        {
            Current_User = user(signup_status);
            crow::response res;
            res.code = 303;
            res.set_header("Location", "/");
            return res;
        } });

    // GET PROFILE PAGE
    // GET PROFILE PAGE
    CROW_ROUTE(app, "/profile/<string>")([](string username)
                                         {
    if (global_login_stats <= 0)
    {
        crow::response res;
        res.code = 303;
        res.set_header("Location", "/login");
        return res;
    }

    if (username[0] != '@')
        username = "@" + username;

    int search_userID = getuserprofile(username);

    if (search_userID <= 0)
    {
        auto message_page = crow::mustache::load("message.html");
        crow::mustache::context ctx({{"error_code", "404"}, {"error_message", "User Not Found"}});
        return crow::response(404, message_page.render(ctx));
    }

    Current_User = user(search_userID);

    crow::mustache::context ctx;
    user currentUser(global_login_stats);

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

        ctx["profile_name"] = Current_User.fullname();
        ctx["profile_handle"] = Current_User.handle();
        string name = Current_User.fullname();
ctx["profile_initials"] = name.size() >= 2 ? name.substr(0,2) : name;
        ctx["profile_bio"] = Current_User.bio();
        ctx["is_verified"] = Current_User.is_verified();

        ctx["profile_location"] = Current_User.location();
        ctx["profile_link"] = Current_User.link();
        ctx["profile_following"] = Current_User.following_count();
        ctx["profile_followers"] = Current_User.followers_count();
        ctx["profile_posts"] = Current_User.posts();

        if (Current_User.created_at().size() >= 10)
            ctx["profile_join_date"] = Current_User.created_at().substr(0, 10);
        else
            ctx["profile_join_date"] = Current_User.created_at();

        if (Current_User.id() == currentUser.id())
            ctx["is_own_profile"] = true;

        ctx["has_posts"] = true;

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
        global_login_stats = userId;

        if (userId > 0)
        {
            crow::response res;
            res.code = 303;
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
        if (global_login_stats <= 0) {
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

        post new_post(content, parent_id);

        crow::response res;
        res.code = 303;
        res.set_header("Location", "/");
        return res; });

    CROW_ROUTE(app, "/updatepost")
        .methods(crow::HTTPMethod::GET, crow::HTTPMethod::POST)([](const crow::request &req)
                                                                {
            if (global_login_stats <= 0)
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

    app.bindaddr("127.0.0.1").port(18080).multithreaded().run();
}
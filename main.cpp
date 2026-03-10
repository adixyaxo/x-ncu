#include "crow.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

int global_login_stats = -1;

class user
{
public:
    int id;
    string fullname = "";
    string email = "";
    string handle = "";
    string role = "";
    string password = "";
    string bio = "";
    bool is_verified = false;
    bool isFound = false;

    user();
    user(int targetUserNo);
    ~user();
};

user::user() {}

// FIX 1: Updated constructor to map to your new users.csv layout
user::user(int targetUserNo)
{
    ifstream file("database/users.csv");

    if (!file.is_open())
        return;

    string line;
    getline(file, line); // Skip header

    while (getline(file, line))
    {
        stringstream ss(line);
        string idStr;

        getline(ss, idStr, ',');

        if (!idStr.empty() && stoi(idStr) == targetUserNo)
        {
            id = targetUserNo;
            getline(ss, handle, ',');
            getline(ss, email, ',');
            getline(ss, fullname, ',');
            getline(ss, role, ',');
            getline(ss, password, ',');
            getline(ss, bio, ',');

            string verified_str;
            getline(ss, verified_str, ',');
            is_verified = (verified_str == "TRUE");

            isFound = true;
            break;
        }
    }
}

user::~user() {}

int getuserprofile(const string &username)
{
    // search username in users.csv and return the user id if found, else return -1
    // handle any file errors with -2 and make sure to skip the header row

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

        // Clean up Windows carriage returns (\r) which often corrupt the last column
        if (!storedPassword.empty() && storedPassword.back() == '\r')
        {
            storedPassword.pop_back();
        }

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
        // Notice there is no 'else' block here anymore!
        // If the email doesn't match, we simply let the loop move to the next row.
    }

    // 6. Loop finished, email was never found
    return 0; // Return 0 here to trigger your "User Not Found" response
}

int registerUser(const string &email, const string &fullName, const string &role, const string &password)
{
    string filePath = "database/users.csv";
    ifstream inFile(filePath);

    int maxId = 0;

    // 1. Open the CSV file to check for existing users and find the highest ID
    if (inFile.is_open())
    {
        string line;

        // Read and discard the first line (the header row)
        getline(inFile, line);

        // Loop through the rows
        while (getline(inFile, line))
        {
            if (line.empty())
                continue; // Skip empty lines

            stringstream ss(line);
            string idStr, storedEmail, dummy;

            // Extract the ID and Email (we don't need the rest for this check)
            getline(ss, idStr, ',');
            getline(ss, storedEmail, ',');

            // Fast-forward through the rest of the columns
            getline(ss, dummy, ','); // fullName
            getline(ss, dummy, ','); // handle
            getline(ss, dummy, ','); // role
            getline(ss, dummy, ','); // password

            // 2. Check if the email is already registered
            if (storedEmail == email)
            {
                inFile.close();
                return -1; // -1 means User Already Exists
            }

            // 3. Keep track of the highest ID so we know what ID to assign next
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
                // Ignore any parsing errors if a row is malformed
            }
        }
        inFile.close();
    }

    // 4. Generate the new user data
    int newId = maxId + 1;

    // Generate a handle from the email (e.g., "aditya@ncuindia.edu" -> "@aditya")
    string handle = "";
    size_t atPos = email.find('@');
    if (atPos != string::npos)
    {
        handle = "@" + email.substr(0, atPos);
    }
    else
    {
        handle = "@user" + to_string(newId); // Fallback handle
    }

    // Check if the file ends with a newline character
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

    // 5. Open the file in Append mode (ios::app) so we don't overwrite existing users
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

    // 6. Append the new user to the end of the file
    outFile << newId << ","
            << email << ","
            << fullName << ","
            << handle << ","
            << role << ","
            << password << "\n";

    outFile.close();

    // 7. Success! Return the new ID
    return newId;
}

user Current_User(global_login_stats);

int main()
{
    crow::SimpleApp app; // define your crow application

    // Explicitly set the base directory for Mustache templates
    crow::mustache::set_base(".");

    // 1. HOME ROUTE
    CROW_ROUTE(app, "/")([]()
                         {
        if (global_login_stats <= 0) 
        {
            crow::response res;
            res.code = 303;
            res.set_header("Location", "/login");
            return res;
        }
        else
        {
            crow::mustache::context ctx;
            ctx["title"] = "HOME | X-NCU";
            
            user currentUser(global_login_stats);

            if (currentUser.isFound) {
                string initials = "U";
                if (currentUser.fullname.length() >= 2) {
                    initials = currentUser.fullname.substr(0, 2);
                }
                ctx["user_initials"] = initials;
                ctx["user_name"] = currentUser.fullname;
                ctx["user_handle"] = currentUser.handle;
            } 

            std::vector<crow::mustache::context> posts_vector;
            std::vector<crow::mustache::context> news_vector;
            
            // --- PARSE POSTS ---
            std::ifstream posts_file("database/posts.csv"); 
            std::string line;
            
            if (posts_file.good()) std::getline(posts_file, line); 
            
            while (std::getline(posts_file, line)) {
                std::stringstream ss(line);
                std::string post_id, user_id, content, parent_id, likes_count, retweets_count, created_at, role;
                
                // Parse up to the new "role" column
                std::getline(ss, post_id, ',');
                std::getline(ss, user_id, ',');
                std::getline(ss, content, ',');
                std::getline(ss, parent_id, ',');
                std::getline(ss, likes_count, ',');
                std::getline(ss, retweets_count, ',');
                std::getline(ss, created_at, ',');
                std::getline(ss, role, ','); 

                // Clean up carriage returns (\r) just in case
                if (!role.empty() && role.back() == '\r') {
                    role.pop_back();
                }
            
                crow::mustache::context post_ctx;
                
                post_ctx["body"] = content;
                post_ctx["likes"] = likes_count;
                post_ctx["reposts"] = retweets_count;
                post_ctx["replies"] = 0; 
                
                // Format timestamp string slightly for UI
                post_ctx["time_ago"] = created_at.substr(0, 10); 
                
                // FIX 2: Look up the actual author of the post dynamically
                user post_author(stoi(user_id));
                
                if(post_author.isFound) {
                    post_ctx["author_name"] = post_author.fullname;
                    post_ctx["author_handle"] = post_author.handle;
                    
                    // Use the role from the post database rather than author lookup
                    post_ctx["author_role"] = role;
                    post_ctx["is_verified"] = post_author.is_verified;
                    
                    // Generate Author Initials dynamically
                    string author_initials = "";
                    if (!post_author.fullname.empty()) {
                        author_initials += post_author.fullname[0];
                        size_t space_pos = post_author.fullname.find(' ');
                        if (space_pos != string::npos && space_pos + 1 < post_author.fullname.length()) {
                            author_initials += post_author.fullname[space_pos + 1];
                        }
                    }
                    if(author_initials.empty()) author_initials = "U";
                    post_ctx["author_initials"] = author_initials;

                    // Set Boolean flags for CSS role pills dynamically checking the DB
                    post_ctx["is_user"] = (role == "student" || role == "Student");
                    post_ctx["is_prof"] = (role == "teacher" || role == "Teacher");
                    post_ctx["is_staff"] = (role == "staff" || role == "Staff");
                }

                posts_vector.push_back(post_ctx); 
            }
            
            ctx["posts"] = std::move(posts_vector);
            
            // --- PARSE NEWS ---
            std::ifstream news_file("database/news.csv");
            if (news_file.good()) std::getline(news_file, line); 
            
            while (std::getline(news_file, line)) {
                std::stringstream ss(line);
                std::string id, headline, category, time_ago, post_count;
                
                std::getline(ss, id, ',');
                std::getline(ss, headline, ',');
                std::getline(ss, category, ',');
                std::getline(ss, time_ago, ',');
                std::getline(ss, post_count, ',');
    
                crow::mustache::context news_ctx;
                news_ctx["headline"] = headline;
                news_ctx["category"] = category;
                news_ctx["time_ago"] = time_ago;
                news_ctx["post_count"] = post_count;
                
                news_vector.push_back(news_ctx);
            }

            ctx["news"] = std::move(news_vector);
            
            auto page = crow::mustache::load("index.html");
            return crow::response(page.render(ctx));
        } });

    // ==========================================
    // 1. STUDENTS FEED ROUTE
    // ==========================================
    CROW_ROUTE(app, "/students")([]()
                                 {
        if (global_login_stats <= 0) {
            crow::response res; res.code = 303; res.set_header("Location", "/login"); return res;
        }

        crow::mustache::context ctx;
        ctx["title"] = "Students | X-NCU";
        
        user currentUser(global_login_stats);
        if (currentUser.isFound) {
            string initials = "U";
            if (currentUser.fullname.length() >= 2) initials = currentUser.fullname.substr(0, 2);
            ctx["user_initials"] = initials;
            ctx["user_name"] = currentUser.fullname;
            ctx["user_handle"] = currentUser.handle;
        } 

        std::vector<crow::mustache::context> posts_vector;
        std::vector<crow::mustache::context> news_vector;
        
        // Parse Students CSV
        std::ifstream posts_file("database/students.csv"); 
        std::string line;
        
        if (posts_file.good()) std::getline(posts_file, line); // Skip header
        
        while (std::getline(posts_file, line)) {
            std::stringstream ss(line);
            std::string post_id, username, full_name, content, likes_count, created_at;
            
            // PostID,Username,FullName,Content,LikesCount,CreatedAt
            std::getline(ss, post_id, ',');
            std::getline(ss, username, ',');
            std::getline(ss, full_name, ',');
            std::getline(ss, content, ',');
            std::getline(ss, likes_count, ',');
            std::getline(ss, created_at, ',');
        
            crow::mustache::context post_ctx;
            post_ctx["body"] = content;
            post_ctx["likes"] = likes_count;
            post_ctx["reposts"] = 0; 
            post_ctx["replies"] = 0; 
            post_ctx["time_ago"] = created_at.substr(0, 10); 
            
            post_ctx["author_name"] = full_name;
            post_ctx["author_handle"] = username;
            
            // Hardcode the role UI for this specific feed
            post_ctx["author_role"] = "Student";
            post_ctx["is_user"] = true;
            post_ctx["is_prof"] = false;
            post_ctx["is_staff"] = false;

            // Generate Initials
            string author_initials = "";
            if (!full_name.empty()) {
                author_initials += full_name[0];
                size_t space_pos = full_name.find(' ');
                if (space_pos != string::npos && space_pos + 1 < full_name.length()) {
                    author_initials += full_name[space_pos + 1];
                }
            }
            if(author_initials.empty()) author_initials = "U";
            post_ctx["author_initials"] = author_initials;

            posts_vector.push_back(post_ctx); 
        }
        ctx["posts"] = std::move(posts_vector);
        
        // --- PARSE NEWS (Reused) ---
        std::ifstream news_file("database/news.csv");
        if (news_file.good()) std::getline(news_file, line); 
        while (std::getline(news_file, line)) {
            std::stringstream ss(line);
            std::string id, headline, category, time_ago, post_count;
            std::getline(ss, id, ','); std::getline(ss, headline, ',');
            std::getline(ss, category, ','); std::getline(ss, time_ago, ','); std::getline(ss, post_count, ',');
            crow::mustache::context news_ctx;
            news_ctx["headline"] = headline; news_ctx["category"] = category;
            news_ctx["time_ago"] = time_ago; news_ctx["post_count"] = post_count;
            news_vector.push_back(news_ctx);
        }
        ctx["news"] = std::move(news_vector);
        
        auto page = crow::mustache::load("index.html");
        return crow::response(page.render(ctx)); });

    // ==========================================
    // 2. TEACHERS FEED ROUTE
    // ==========================================
    CROW_ROUTE(app, "/teachers")([]()
                                 {
        if (global_login_stats <= 0) { crow::response res; res.code = 303; res.set_header("Location", "/login"); return res; }

        crow::mustache::context ctx; ctx["title"] = "Teachers | X-NCU";
        user currentUser(global_login_stats);
        if (currentUser.isFound) {
            string initials = "U"; if (currentUser.fullname.length() >= 2) initials = currentUser.fullname.substr(0, 2);
            ctx["user_initials"] = initials; ctx["user_name"] = currentUser.fullname; ctx["user_handle"] = currentUser.handle;
        } 

        std::vector<crow::mustache::context> posts_vector;
        std::vector<crow::mustache::context> news_vector;
        
        std::ifstream posts_file("database/teachers.csv"); std::string line;
        if (posts_file.good()) std::getline(posts_file, line); 
        
        while (std::getline(posts_file, line)) {
            std::stringstream ss(line);
            std::string post_id, username, full_name, content, likes_count, created_at;
            std::getline(ss, post_id, ','); std::getline(ss, username, ','); std::getline(ss, full_name, ',');
            std::getline(ss, content, ','); std::getline(ss, likes_count, ','); std::getline(ss, created_at, ',');
        
            crow::mustache::context post_ctx;
            post_ctx["body"] = content; post_ctx["likes"] = likes_count; post_ctx["reposts"] = 0; post_ctx["replies"] = 0; 
            post_ctx["time_ago"] = created_at.substr(0, 10); post_ctx["author_name"] = full_name; post_ctx["author_handle"] = username;
            
            // Hardcode the role UI for Teachers
            post_ctx["author_role"] = "Teacher";
            post_ctx["is_user"] = false;
            post_ctx["is_prof"] = true;
            post_ctx["is_staff"] = false;

            string author_initials = "";
            if (!full_name.empty()) {
                author_initials += full_name[0]; size_t space_pos = full_name.find(' ');
                if (space_pos != string::npos && space_pos + 1 < full_name.length()) author_initials += full_name[space_pos + 1];
            }
            if(author_initials.empty()) author_initials = "U";
            post_ctx["author_initials"] = author_initials;
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
    // 3. STAFF FEED ROUTE
    // ==========================================
    CROW_ROUTE(app, "/staff")([]()
                              {
        if (global_login_stats <= 0) { crow::response res; res.code = 303; res.set_header("Location", "/login"); return res; }

        crow::mustache::context ctx; ctx["title"] = "Staff | X-NCU";
        user currentUser(global_login_stats);
        if (currentUser.isFound) {
            string initials = "U"; if (currentUser.fullname.length() >= 2) initials = currentUser.fullname.substr(0, 2);
            ctx["user_initials"] = initials; ctx["user_name"] = currentUser.fullname; ctx["user_handle"] = currentUser.handle;
        } 

        std::vector<crow::mustache::context> posts_vector;
        std::vector<crow::mustache::context> news_vector;
        
        std::ifstream posts_file("database/staff.csv"); std::string line;
        if (posts_file.good()) std::getline(posts_file, line); 
        
        while (std::getline(posts_file, line)) {
            std::stringstream ss(line);
            std::string post_id, username, full_name, content, likes_count, created_at;
            std::getline(ss, post_id, ','); std::getline(ss, username, ','); std::getline(ss, full_name, ',');
            std::getline(ss, content, ','); std::getline(ss, likes_count, ','); std::getline(ss, created_at, ',');
        
            crow::mustache::context post_ctx;
            post_ctx["body"] = content; post_ctx["likes"] = likes_count; post_ctx["reposts"] = 0; post_ctx["replies"] = 0; 
            post_ctx["time_ago"] = created_at.substr(0, 10); post_ctx["author_name"] = full_name; post_ctx["author_handle"] = username;
            
            // Hardcode the role UI for Staff
            post_ctx["author_role"] = "Staff";
            post_ctx["is_user"] = false;
            post_ctx["is_prof"] = false;
            post_ctx["is_staff"] = true;

            string author_initials = "";
            if (!full_name.empty()) {
                author_initials += full_name[0]; size_t space_pos = full_name.find(' ');
                if (space_pos != string::npos && space_pos + 1 < full_name.length()) author_initials += full_name[space_pos + 1];
            }
            if(author_initials.empty()) author_initials = "U";
            post_ctx["author_initials"] = author_initials;
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

    // 2. GET LOGIN PAGE
    CROW_ROUTE(app, "/login")([]()
                              { 
        auto variable_page = crow::mustache::load("login.html"); 
        return variable_page.render(); });

    // 3. GET SIGNUP PAGE
    CROW_ROUTE(app, "/signup")([]()
                               { 
        auto variable_page = crow::mustache::load("signup.html"); 
        return variable_page.render(); });

    // 4. POST REGISTER DATA
    CROW_ROUTE(app, "/register").methods(crow::HTTPMethod::POST)([](const crow::request &req)
                                                                 {
        crow::query_string params("?" + req.body);

        std::string name = params.get("name") ? params.get("name") : "";
        std::string email = params.get("email") ? params.get("email") : "";
        std::string role = params.get("role") ? params.get("role") : "";
        std::string password = params.get("password") ? params.get("password") : "";

        if (name.empty() || email.empty() || role.empty() || password.empty()) {
            return crow::response(400, "Error: All fields are required!");
        }

        int signup_status = registerUser(email, name, role, password);
        global_login_stats = signup_status;

        if (signup_status == -1) {
            auto message_page = crow::mustache::load("message.html");
            crow::mustache::context ctx({{"error_code", "422"}, {"error_message", "User already exists"}});
            return crow::response(422, message_page.render(ctx));
        }
        else {
            // SUCCESSFUL REGISTRATION - Force a GET request to the feed
            Current_User = user(signup_status);
            crow::response res;
            res.code = 303; 
            res.set_header("Location", "/");
            return res;
        } });

    // 5. GET PROFILE PAGE
    CROW_ROUTE(app, "/profile/<string>")([](string username)
                                         {
        if (global_login_stats <= 0) 
        {
            crow::response res;
            res.code = 303;
            res.set_header("Location", "/login");
            return res; 
        }
        else 
        { 
            username = "@" + username;
            crow::mustache::context ctx;
            ctx["title"] = Current_User.handle;

            ctx["user_initials"] = Current_User.fullname.substr(0, 2);
            ctx["user_name"] = Current_User.fullname;
            ctx["user_handle"] = Current_User.handle;
            
            ctx["profile_name"] = Current_User.fullname;
            ctx["profile_handle"] = Current_User.handle;
            ctx["profile_initials"] = Current_User.fullname.substr(0, 2);
            ctx["profile_post_count"] = 0;
            ctx["profile_bio"] = Current_User.bio;
            ctx["profile_location"] = Current_User.location;
            ctx["profile_link"] = Current_User.link;
            ctx["profile_join_date"] = Current_User.join_date;
            ctx["following_count"] = Current_User.following_count;
            ctx["followers_count"] = Current_User.followers_count;
            ctx["is_verified"] = Current_User.is_verified;
            
            ctx["is_own_profile"] = true; 
            ctx["has_posts"] = false;     

            auto profile_page = crow::mustache::load("profile.html").render(ctx); 
            return crow::response(profile_page); 
        } });

    // 6. POST AUTHENTICATION DATA
    CROW_ROUTE(app, "/auth").methods(crow::HTTPMethod::POST)([](const crow::request &req)
                                                             {
        crow::query_string params("?" + req.body);
        std::string email = params.get("email") ? params.get("email") : "";
        std::string password = params.get("password") ? params.get("password") : "";
        
        int userId = authenticateUser(email, password);
        global_login_stats = userId;
        
        if (userId > 0) // Valid IDs are positive numbers
        {
            // SUCCESSFUL LOGIN - Force a GET request to the feed
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

    // set the port, set the app to run on multiple threads, and run the app
    app.bindaddr("127.0.0.1").port(18080).multithreaded().run();
}
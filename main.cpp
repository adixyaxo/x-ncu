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
    bool isFound = false;

    user();                 // Default constructor
    user(int targetUserNo); // Parameterized constructor
    ~user();
};

// FIX 1: You must define the default constructor so the linker doesn't fail
user::user() {}

user::user(int targetUserNo)
{
    ifstream file("database/users.csv");

    if (!file.is_open())
    {
        cerr << "Error: Could not open database/users.csv." << endl;
        // FIX 2: Constructors cannot return a value. Just use `return;`
        return;
    }

    string line;
    getline(file, line);

    while (getline(file, line))
    {
        stringstream ss(line);
        string idStr;

        getline(ss, idStr, ',');

        if (!idStr.empty() && stoi(idStr) == targetUserNo)
        {
            // FIX 3: Removed "user." prefixes. Inside a class, just use the variable name directly.
            id = targetUserNo;
            getline(ss, email, ',');
            getline(ss, fullname, ','); // FIX 4: Changed to lowercase 'n' to match your class variable
            getline(ss, handle, ',');
            getline(ss, role, ',');
            getline(ss, password, ',');

            if (!password.empty() && password.back() == '\r')
            {
                password.pop_back();
            }

            isFound = true;
            break;
        }
    }
}

user::~user()
{
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
        string idStr, email, fullName, handle, role, storedPassword;

        // 4. Extract each column up to the comma
        getline(ss, idStr, ',');
        getline(ss, email, ',');
        getline(ss, fullName, ',');
        getline(ss, handle, ',');
        getline(ss, role, ',');
        getline(ss, storedPassword, ',');

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

    // If the file was completely empty/new, you might want to write the header here.
    // Assuming the header already exists based on your authenticateUser function.

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
        if (global_login_stats <= 0) // If no user is logged in
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
            // Fetch the logged-in user's info dynamically from the CSV
            user currentUser(global_login_stats);

            if (currentUser.isFound) {
                // Create a basic 2-letter initial from their full name
                string initials = "U";
                if (currentUser.fullname.length() >= 2) {
                    initials = currentUser.fullname.substr(0, 2);
                }

                ctx["user_initials"] = initials;
                ctx["user_name"] = currentUser.fullname;
                ctx["user_handle"] = currentUser.handle;
            } else {
                ctx["user_initials"] = "?";
                ctx["user_name"] = "Unknown User";
                ctx["user_handle"] = "@error";
            }

            // Feed Posts Data
            std::ifstream posts_file("posts.csv"); 
            std::string line;
            
            // Skip the header row
            if (posts_file.good()) std::getline(posts_file, line); 
            
            while (std::getline(posts_file, line)) {
                std::stringstream ss(line);
                
                // Create variables for all 7 columns in the posts CSV
                std::string post_id, user_id, content, parent_id, likes_count, retweets_count, created_at;
                
                // Extract each column strictly in the order they appear
                std::getline(ss, post_id, ',');
                std::getline(ss, user_id, ',');
                std::getline(ss, content, ',');
                std::getline(ss, parent_id, ',');
                std::getline(ss, likes_count, ',');
                std::getline(ss, retweets_count, ',');
                std::getline(ss, created_at, ',');
            
                crow::mustache::context post_ctx;
                
                // Map the extracted values to the Mustache template variables
                post_ctx["body"] = content;
                post_ctx["likes"] = likes_count;
                post_ctx["reposts"] = retweets_count;
                post_ctx["time_ago"] = created_at; 
                
                // Since this file only has UserID, we create a placeholder for the UI
                // In a full database setup, you would JOIN the Users and Posts tables to get the real name
                post_ctx["author_name"] = "User " + user_id;
                post_ctx["author_handle"] = "@user_" + user_id;
            
                // Optional: Add logic to check if it's a reply
                post_ctx["is_reply"] = (parent_id != "0");
            
                // Push to your vector
                posts_vector.push_back(post_ctx); 
            }
                                    ctx["news"] = std::move(news_vector);
                    std::ifstream news_file("./database/news.csv");
                                    if (news_file.good()) std::getline(news_file, line); // skip header
                            
                                    while (std::getline(news_file, line)) {
                                        std::stringstream ss(line);
                                        std::string id, headline, category, time_ago, post_count;
                                        
                                        // NewsID,Headline,Category,TimeAgo,PostCount
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

    // 4. GET PROFILE PAGE
    CROW_ROUTE(app, "/profile")([]()
                                {
        if (global_login_stats <= 0) // If no user is logged in
        {
        crow::response res;
        res.code = 303;
        res.set_header("Location", "/login");
        return res; }

        else { crow::mustache::context ctx;
                                    ctx["title"] = Current_User.handle;

                                    // 1. The viewer (Left Sidebar)
                                    ctx["user_initials"] = "AD";
                                    ctx["user_name"] = "Aditya Dagar";
                                    ctx["user_handle"] = "@adixyaxo";
                                    
                                    // 2. The profile being viewed (Main Content)
                                    ctx["profile_name"] = "Aditya Dagar";
                                    ctx["profile_handle"] = "@adixyaxo";
                                    ctx["profile_initials"] = "AD";
                                    ctx["profile_post_count"] = 0;
                                    ctx["profile_bio"] = "Computer Science Undergrad @ NorthCap University. Building cool stuff. 🚀";
                                    ctx["profile_location"] = "Gurugram, India";
                                    ctx["profile_link"] = "github.com/adixyaxo";
                                    ctx["profile_join_date"] = "Jan 2026";
                                    ctx["following_count"] = 12;
                                    ctx["followers_count"] = 8;
                                    
                                    // 3. Logic flags
                                    ctx["is_own_profile"] = true; // Shows "Edit profile" instead of "Follow"
                                    ctx["has_posts"] = false;     // Triggers the "No posts yet" empty state
                                    


                                    








        auto profile_page = crow::mustache::load("profile.html").render(ctx); 
        return crow::response(profile_page); } });

    // 5. POST AUTHENTICATION DATA
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
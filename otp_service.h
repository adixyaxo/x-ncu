#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <unordered_map>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

using boost::asio::ip::tcp;

namespace otp {

    // ==========================================
    // CONFIGURATION VARIABLES
    // ==========================================
    // Time limit for OTP validity in minutes
    const int TIME_LIMIT_MINUTES = 5;                           
    // Number of digits in the OTP
    const int OTP_LENGTH = 6;                                   
    // Subject line of the email
    const std::string OTP_SUBJECT = "X-NCU || Your Secure OTP Code";     
    // Body of the email. Use {OTP} and {TIME_LIMIT} as placeholders.
    const std::string OTP_TEXT_TEMPLATE = "Your OTP is: {OTP}. It will expire in {TIME_LIMIT} minutes."; 
    
    // GMAIL ACCOUNT DETAILS
    const std::string GMAIL_ACCOUNT = "aditya25csu016@ncuindia.edu";   // The Gmail address to send from
    const std::string SMTP_ACCOUNT_KEY = "jdsc jexd klga hwkk";   // The App Password (not normal password)
    // ==========================================

    static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

    // Helper to base64 encode strings for SMTP AUTH LOGIN
    inline std::string base64_encode(const std::string& in) {
        std::string out;
        int val = 0, valb = -6;
        for (unsigned char c : in) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                out.push_back(base64_chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) out.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
        while (out.size() % 4) out.push_back('=');
        return out;
    }

    struct OTPRecord {
        std::string otp;
        std::chrono::steady_clock::time_point timestamp;
    };

    class OTPService {
    private:
        std::unordered_map<std::string, OTPRecord> otp_store;
        std::mutex mtx;

        std::string generateRandomDigits(int length) {
            std::string result;
            result.reserve(length);
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 9);
            for (int i = 0; i < length; ++i) {
                result += std::to_string(dis(gen));
            }
            return result;
        }

        // Connects to Gmail SMTP and sends raw email via TLS on port 465
        bool sendMailRaw(const std::string& to_email, const std::string& subject, const std::string& body) {
            try {
                boost::asio::io_context io_context;
                boost::asio::ssl::context ssl_context(boost::asio::ssl::context::tlsv12_client);
                ssl_context.set_default_verify_paths();

                tcp::resolver resolver(io_context);
                auto endpoints = resolver.resolve("smtp.gmail.com", "465");

                boost::asio::ssl::stream<tcp::socket> socket(io_context, ssl_context);
                boost::asio::connect(socket.lowest_layer(), endpoints);
                socket.handshake(boost::asio::ssl::stream_base::client);

                // Helper to read exactly one response block
                auto read_response = [&socket]() {
                    char buf[1024];
                    boost::system::error_code error;
                    size_t length = socket.read_some(boost::asio::buffer(buf), error);
                    std::string resp(buf, length);
                    return resp;
                };

                // Helper to send a command and read the response
                auto send_cmd = [&socket, &read_response](const std::string& cmd) {
                    boost::asio::write(socket, boost::asio::buffer(cmd + "\r\n"));
                    return read_response();
                };

                // SMTP sequence
                read_response(); // Consume initial 220 banner
                
                send_cmd("EHLO localhost");
                send_cmd("AUTH LOGIN");
                send_cmd(base64_encode(GMAIL_ACCOUNT));
                send_cmd(base64_encode(SMTP_ACCOUNT_KEY));
                send_cmd("MAIL FROM:<" + GMAIL_ACCOUNT + ">");
                send_cmd("RCPT TO:<" + to_email + ">");
                send_cmd("DATA");

                // Send email RFC 822 format
                std::string email_data = 
                    "To: " + to_email + "\r\n" +
                    "From: " + GMAIL_ACCOUNT + "\r\n" +
                    "Subject: " + subject + "\r\n" +
                    "\r\n" +
                    body + "\r\n.";

                send_cmd(email_data);
                send_cmd("QUIT");
                
                // Close gracefully
                boost::system::error_code ec;
                socket.shutdown(ec);

                return true;
            } catch (std::exception& e) {
                std::cerr << "[OTP Service] Failed to send email: " << e.what() << "\n";
                return false;
            }
        }

    public:
        // Singleton pattern
        static OTPService& getInstance() {
            static OTPService instance;
            return instance;
        }

        // Generates an OTP, saves it in memory, and triggers email
        bool generateAndSendOTP(const std::string& to_email) {
            std::string otp_code = generateRandomDigits(OTP_LENGTH);
            
            // Format the message template
            std::string body = OTP_TEXT_TEMPLATE;
            size_t otp_pos = body.find("{OTP}");
            if (otp_pos != std::string::npos) {
                body.replace(otp_pos, 5, otp_code);
            }
            size_t time_pos = body.find("{TIME_LIMIT}");
            if (time_pos != std::string::npos) {
                body.replace(time_pos, 12, std::to_string(TIME_LIMIT_MINUTES));
            }

            // Send Email blocks thread (consider calling this method via std::async or crowd thread)
            if (!sendMailRaw(to_email, OTP_SUBJECT, body)) {
                return false;
            }

            // Lock and save OTP securely
            std::lock_guard<std::mutex> lock(mtx);
            otp_store[to_email] = {otp_code, std::chrono::steady_clock::now()};
            return true;
        }

        // Verifies the OTP, checks against the config time limit
        bool verifyOTP(const std::string& email, const std::string& input_otp) {
            std::lock_guard<std::mutex> lock(mtx);
            auto it = otp_store.find(email);
            
            // Email not found or no OTP exists
            if (it == otp_store.end()) {
                return false;
            }

            auto now = std::chrono::steady_clock::now();
            auto time_elapsed = std::chrono::duration_cast<std::chrono::minutes>(now - it->second.timestamp).count();

            // Time limit validation
            if (time_elapsed > TIME_LIMIT_MINUTES) {
                otp_store.erase(it); // Expired
                return false;
            }

            // Success matching
            if (it->second.otp == input_otp) {
                otp_store.erase(it); // Verify success, invalidate immediately to prevent reuse
                return true;
            }

            return false;
        }
    };

} // namespace otp

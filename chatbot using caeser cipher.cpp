#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <curl/curl.h>
#include<nlohmann/json.hpp>
using json=nlohmann::json;
using namespace std;
// The CaesarCipher class is the base class that handles the core encryption logic.
class CaesarCipher {
protected:
    int shiftKey;

public:
    CaesarCipher() {
        shiftKey = 3;
    }

    CaesarCipher(int key) {
        shiftKey = key;
    }
//setter
    void setShiftKey(int key) {
        shiftKey = key;
    }
//getter
    int getShiftKey() const {
        return shiftKey;
    }

    virtual string encryptMessage(string text) {
        string result = text;
        for (size_t i = 0; i < result.size(); i++) {
            char c = result[i];
            if (isupper(c)) {
                result[i] = char((((c - 'A') + shiftKey) % 26 + 26) % 26 + 'A');
            } else if (islower(c)) {
                result[i] = char((((c - 'a') + shiftKey) % 26 + 26) % 26 + 'a');
            }
        }
        return result;
    }

    string encryptMessage(string text, int customKey) {
        string result = text;
        for (size_t i = 0; i < result.size(); i++) {
            char c = result[i];
            if (isupper(c)) {
                result[i] = char((((c - 'A') + customKey) % 26 + 26) % 26 + 'A');
            } else if (islower(c)) {
                result[i] = char((((c - 'a') + customKey) % 26 + 26) % 26 + 'a');
            }
        }
        return result;
    }

    virtual string decryptMessage(string text) {
        string result = text;
        for (size_t i = 0; i < result.size(); i++) {
            char c = result[i];
            if (isupper(c)) {
                result[i] = char((((c - 'A') - shiftKey) % 26 + 26) % 26 + 'A');
            } else if (islower(c)) {
                result[i] = char((((c - 'a') - shiftKey) % 26 + 26) % 26 + 'a');
            }
        }
        return result;
    }


    virtual ~CaesarCipher() {}
};

// The Chatbot class is derived from CaesarCipher. It manages the conversation
// with the user, sends each message to the Gemini API configured as a legal
// information assistant, and stores every exchange in encrypted and
// decrypted files using the inherited cipher functions.
class Chatbot : public CaesarCipher {
private:
    string userName;
    string apiKey;
    //for writing 
    ofstream encryptedFile;
    ofstream decryptedFile;

    static size_t writeCallback(void *contents, size_t size, size_t nmemb, string *output) {
        size_t totalSize = size * nmemb;
        output->append((char *)contents, totalSize);
        return totalSize;
    }

    string escapeJsonString(string s) {
        string result;
        for (size_t i = 0; i < s.size(); i++) {
            char c = s[i];
            if (c == '"' || c == '\\') {
                result += '\\';
                result += c;
            } else if (c == '\n') {
                result += "\\n";
            } else {
                result += c;
            }
        }
        return result;
    }

    string extractResponseText(string jsonResponse)
{
    try
    {
        json response = json::parse(jsonResponse);

        if (response.contains("error"))
        {
            return "API Error: " +
                   response["error"]["message"].get<string>();
        }

        return response["candidates"][0]
                       ["content"]["parts"][0]
                       ["text"]
            .get<string>();
    }
    catch (...)
    {
        return "Error: Unable to read Gemini response.";
    }
}

    string getCurrentDate() {
        time_t now = time(0);
        tm *ltm = localtime(&now);
        string date = to_string(1900 + ltm->tm_year) + "-" +
                      to_string(1 + ltm->tm_mon) + "-" +
                      to_string(ltm->tm_mday);
        return date;
    }

    string getCurrentTime() {
        time_t now = time(0);
        tm *ltm = localtime(&now);
        string time_str = to_string(ltm->tm_hour) + ":" +
                           to_string(ltm->tm_min) + ":" +
                           to_string(ltm->tm_sec);
        return time_str;
}


    void saveChatRecord(string userMessage, string aiResponse) {
        string date = getCurrentDate();
        string time_str = getCurrentTime();

        string encryptedUserMsg = encryptMessage(userMessage);
        string encryptedAiMsg = encryptMessage(aiResponse);

        encryptedFile << "User Name: " << userName << endl;
        encryptedFile << "Date: " << date << endl;
        encryptedFile << "Time: " << time_str << endl;
        encryptedFile << "User Message: " << encryptedUserMsg << endl;
        encryptedFile << "AI Response: " << encryptedAiMsg << endl;
        encryptedFile << "----------------------------------------" << endl;

        string decryptedUserMsg = decryptMessage(encryptedUserMsg);
        string decryptedAiMsg = decryptMessage(encryptedAiMsg);

        decryptedFile << "User Name: " << userName << endl;
        decryptedFile << "Date: " << date << endl;
        decryptedFile << "Time: " << time_str << endl;
        decryptedFile << "User Message: " << decryptedUserMsg << endl;
        decryptedFile << "AI Response: " << decryptedAiMsg << endl;
        decryptedFile << "----------------------------------------" << endl;
    }

public:
    Chatbot(string name, int key, string userApiKey) : CaesarCipher(key) {
        userName = name;
        apiKey = userApiKey;
    }

    void openFiles() {
        encryptedFile.open("C:\\Users\\CC\\OneDrive\\Desktop\\MY PROJECT\\Memouna_Encrypted.txt", ios::app);
        decryptedFile.open("C:\\Users\\CC\\OneDrive\\Desktop\\MY PROJECT\\Memouna_Decrypted.txt", ios::app);

        if (!encryptedFile || !decryptedFile) {
            cout << "Error: Could not create/open files. Please check the folder path." << endl;
        }
    }

    void closeFiles() {
        if (encryptedFile.is_open()) encryptedFile.close();
        if (decryptedFile.is_open()) decryptedFile.close();
    }
    
    virtual string generateResponse(string userMessage) {
        CURL *curl = curl_easy_init();
        string responseBuffer;

        if (!curl) {
            return "Error: Could not initialize network connection.";
        }
     string escapedMessage = escapeJsonString(userMessage);

string jsonBody =
"{\"contents\": [{\"parts\": [{\"text\": \"" + escapedMessage + "\"}]}]}";

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "content-type: application/json");

        string url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent?key=" + apiKey;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBody.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

        CURLcode res = curl_easy_perform(curl);
 

        string finalText;
        if (res != CURLE_OK) {
            finalText = "Network error: could not reach the legal AI service. Please check your internet connection.";
        } else {
            finalText = extractResponseText(responseBuffer);
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        return finalText;
    }

    void startConversation() {
        string userMessage;
        cout << "Ask Gemini" << endl;

        while (true) {
            cout << userName << ": ";
            getline(cin, userMessage);

            string lowerMsg = userMessage;
            for (size_t i = 0; i < lowerMsg.size(); i++) lowerMsg[i] = tolower(lowerMsg[i]);
            if (lowerMsg.find("bye") != string::npos || lowerMsg.find("exit") != string::npos) {
                cout << "Chatbot: Goodbye! Remember this is general information, not formal legal advice." << endl;
                saveChatRecord(userMessage, "Goodbye! Remember this is general information, not formal legal advice.");
                break;
            }

            string aiResponse = generateResponse(userMessage);
            cout << "Chatbot: " << aiResponse << endl;

            saveChatRecord(userMessage, aiResponse);
        }
    }
};

int main() {
    string userName;
    int key;

    cout << "==========================================" << endl;
    cout << "         AI Assistance               " << endl;
    cout << "==========================================" << endl;

    string apikey="";
    cout << "Enter your name: ";
    getline(cin, userName);

    /*cout << "Enter shift key for encryption (1-25): ";
    cin >> key;
    cin.ignore();*/

    curl_global_init(CURL_GLOBAL_ALL);

    Chatbot bot(userName, key, apikey);
    bot.openFiles();
   // CaesarCipher *cipherPtr = &bot;
   // cipherPtr->displayCipherInfo();

    bot.startConversation();
    bot.closeFiles();

    curl_global_cleanup();

    cout << "\nChat history has been saved successfully." << endl;
    return 0;
}
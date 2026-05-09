/*
Booked C++ backend - Windows friendly version
Stores book data in std::map<string, Book>, saves it to data/books.tsv,
and exposes tiny HTTP endpoints for the frontend.

Endpoints:
GET  /api/books
POST /api/books  with form fields: title, author, rating, month
GET  /api/recommendations

Compile on Windows with MinGW/MSYS2:
g++ -std=c++17 main.cpp -o booked_server.exe -lws2_32
*/

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "Ws2_32.lib")
#else
  #include <arpa/inet.h>
  #include <csignal>
  #include <netinet/in.h>
  #include <sys/socket.h>
  #include <unistd.h>
#endif

using namespace std;

#ifdef _WIN32
using SocketType = SOCKET;
#else
using SocketType = int;
#endif

struct Book {
    string title;
    string author;
    int rating;
    string month;
};

map<string, Book> libraryByTitle;
const string DATA_FILE = "data/books.tsv";

void closeSocket(SocketType socketHandle) {
#ifdef _WIN32
    closesocket(socketHandle);
#else
    close(socketHandle);
#endif
}

string urlDecode(const string& value) {
    string result;
    for (size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '%' && i + 2 < value.size()) {
            string hex = value.substr(i + 1, 2);
            char decoded = static_cast<char>(strtol(hex.c_str(), nullptr, 16));
            result += decoded;
            i += 2;
        } else if (value[i] == '+') {
            result += ' ';
        } else {
            result += value[i];
        }
    }
    return result;
}

string escapeJson(const string& text) {
    string out;
    for (char c : text) {
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else out += c;
    }
    return out;
}

map<string, string> parseFormBody(const string& body) {
    map<string, string> fields;
    stringstream stream(body);
    string pair;
    while (getline(stream, pair, '&')) {
        size_t equals = pair.find('=');
        if (equals != string::npos) {
            string key = urlDecode(pair.substr(0, equals));
            string value = urlDecode(pair.substr(equals + 1));
            fields[key] = value;
        }
    }
    return fields;
}

void loadBooks() {
    filesystem::create_directories("data");
    ifstream input(DATA_FILE);
    if (!input) return;

    string line;
    while (getline(input, line)) {
        stringstream ss(line);
        Book book;
        string ratingText;
        getline(ss, book.title, '\t');
        getline(ss, book.author, '\t');
        getline(ss, ratingText, '\t');
        getline(ss, book.month, '\t');

        if (!book.title.empty()) {
            try { book.rating = stoi(ratingText); }
            catch (...) { book.rating = 0; }
            libraryByTitle[book.title] = book;
        }
    }
}

void saveBooks() {
    filesystem::create_directories("data");
    ofstream output(DATA_FILE);
    for (const auto& entry : libraryByTitle) {
        const Book& book = entry.second;
        output << book.title << '\t'
               << book.author << '\t'
               << book.rating << '\t'
               << book.month << '\n';
    }
}

string booksJson() {
    string json = "[";
    bool first = true;
    for (const auto& entry : libraryByTitle) {
        const Book& book = entry.second;
        if (!first) json += ",";
        first = false;
        json += "{";
        json += "\"title\":\"" + escapeJson(book.title) + "\",";
        json += "\"author\":\"" + escapeJson(book.author) + "\",";
        json += "\"rating\":" + to_string(book.rating) + ",";
        json += "\"month\":\"" + escapeJson(book.month) + "\"";
        json += "}";
    }
    json += "]";
    return json;
}

string recommendationsJson() {
    string lovedTitles;
    double totalRating = 0;
    int count = 0;

    for (const auto& entry : libraryByTitle) {
        const Book& book = entry.second;
        totalRating += book.rating;
        count++;

        if (book.rating >= 4) {
            string lower = book.title;
            transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            lovedTitles += lower + " ";
        }
    }

    vector<string> recs;
    if (lovedTitles.find("cat") != string::npos) {
        recs = {"The Travelling Cat Chronicles", "The Cat Who Saved Books", "The Wild Robot"};
    } else if (lovedTitles.find("latte") != string::npos || lovedTitles.find("psalm") != string::npos) {
        recs = {"Bookshops & Bonedust", "The Long Way to a Small, Angry Planet", "The House in the Cerulean Sea"};
    } else if (count > 0 && totalRating / count >= 4.5) {
        recs = {"Howl's Moving Castle", "Ella Enchanted", "The Girl Who Drank the Moon"};
    } else {
        recs = {"Legends & Lattes", "A Psalm for the Wild-Built", "The Hobbit"};
    }

    string json = "[";
    for (size_t i = 0; i < recs.size(); ++i) {
        if (i) json += ",";
        json += "\"" + escapeJson(recs[i]) + "\"";
    }
    json += "]";
    return json;
}

string httpResponse(const string& body, const string& contentType = "application/json", const string& status = "200 OK") {
    return "HTTP/1.1 " + status + "\r\n"
        + "Content-Type: " + contentType + "\r\n"
        + "Access-Control-Allow-Origin: *\r\n"
        + "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        + "Access-Control-Allow-Headers: Content-Type\r\n"
        + "Content-Length: " + to_string(body.size()) + "\r\n"
        + "Connection: close\r\n\r\n"
        + body;
}

void handleClient(SocketType clientSocket) {
    char buffer[8192] = {0};

#ifdef _WIN32
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
#else
    int bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
#endif

    if (bytesRead <= 0) {
        closeSocket(clientSocket);
        return;
    }

    string request(buffer, bytesRead);
    stringstream requestLine(request.substr(0, request.find("\r\n")));
    string method, path, version;
    requestLine >> method >> path >> version;

    string response;

    if (method == "OPTIONS") {
        response = httpResponse("");
    } else if (method == "GET" && path == "/api/books") {
        response = httpResponse(booksJson());
    } else if (method == "GET" && path == "/api/recommendations") {
        response = httpResponse(recommendationsJson());
    } else if (method == "POST" && path == "/api/books") {
        size_t bodyStart = request.find("\r\n\r\n");
        string body = bodyStart == string::npos ? "" : request.substr(bodyStart + 4);
        auto fields = parseFormBody(body);

        if (fields["title"].empty() || fields["author"].empty()) {
            response = httpResponse("{\"error\":\"Title and author are required.\"}", "application/json", "400 Bad Request");
        } else {
            Book book;
            book.title = fields["title"];
            book.author = fields["author"];
            book.month = fields["month"].empty() ? "January" : fields["month"];

            try { book.rating = max(1, min(5, stoi(fields["rating"]))); }
            catch (...) { book.rating = 5; }

            libraryByTitle[book.title] = book;
            saveBooks();
            response = httpResponse("{\"saved\":true}");
        }
    } else {
        response = httpResponse("{\"error\":\"Not found.\"}", "application/json", "404 Not Found");
    }

#ifdef _WIN32
    send(clientSocket, response.c_str(), static_cast<int>(response.size()), 0);
#else
    send(clientSocket, response.c_str(), response.size(), 0);
#endif

    closeSocket(clientSocket);
}

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    int startupResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (startupResult != 0) {
        cerr << "WSAStartup failed.\n";
        return 1;
    }
#else
    signal(SIGPIPE, SIG_IGN);
#endif

    loadBooks();

    SocketType serverSocket = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
    if (serverSocket == INVALID_SOCKET) {
#else
    if (serverSocket < 0) {
#endif
        cerr << "Could not create socket.\n";
        return 1;
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        cerr << "Could not bind to port 8080. Is another server running?\n";
        closeSocket(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 10) < 0) {
        cerr << "Could not listen for connections.\n";
        closeSocket(serverSocket);
        return 1;
    }

    cout << "Booked backend is running at http://localhost:8080\n";
    cout << "Open frontend/index.html in your browser.\n";
    cout << "Press Ctrl+C to stop.\n";

    while (true) {
        sockaddr_in clientAddress{};
        socklen_t clientSize = sizeof(clientAddress);
        SocketType clientSocket = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddress), &clientSize);

#ifdef _WIN32
        if (clientSocket != INVALID_SOCKET) handleClient(clientSocket);
#else
        if (clientSocket >= 0) handleClient(clientSocket);
#endif
    }

    closeSocket(serverSocket);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}

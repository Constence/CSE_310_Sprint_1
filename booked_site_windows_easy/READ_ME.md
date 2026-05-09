# Project Title

Booked

## Project Description

Booked is a caterpillar-themed reading tracker built with a C++ backend and a custom HTML/CSS/JavaScript frontend. The application allows users to track books they have read, store author names, give ratings out of five, and visualize reading progress throughout the year. The project uses STL data structures such as maps and vectors to manage and organize saved book data. The interface uses a green garden-inspired caterpillar theme with progress tracking and recommendation features.

## Instructions for Build and Use

### Steps to build and/or run the software:

1. Download and install MSYS2 from:
   https://www.msys2.org/

2. Open the "MSYS2 UCRT64" terminal.

3. Install g++ using:
   pacman -S mingw-w64-ucrt-x86_64-gcc

4. Open the project folder in Visual Studio Code.

5. In the MSYS2 UCRT64 terminal, navigate to the backend folder:
   cd "/c/path/to/project/backend"

6. Compile the backend:
   g++ -std=c++17 main.cpp -o booked_server.exe -lws2_32

7. Run the backend:
   ./booked_server.exe

8. Open frontend/index.html in a browser or with Live Server.

### Instructions for using the software:

1. Start the backend server before opening the frontend.

2. Open the website frontend.

3. Enter a book title, author, rating, and month finished.

4. Press the "Munch This Book" button to save the book.

5. View saved books, reading statistics, yearly progress, and recommendations.

## Development Environment

To recreate the development environment, you need the following software and/or libraries:

* Visual Studio Code
* MSYS2 UCRT64
* g++ compiler with C++17 support
* HTML5
* CSS3
* JavaScript
* Windows 10 or Windows 11

## Useful Websites to Learn More

I found these websites useful in developing this software:

* https://www.msys2.org/
* https://cplusplus.com/
* https://developer.mozilla.org/
* https://www.w3schools.com/
* https://en.cppreference.com/

## Future Work

The following items I plan to improve or add in the future:

* [ ] Add user accounts and login support
* [ ] Add a real AI-powered recommendation system
* [ ] Add book cover image uploads
* [ ] Add reading streak tracking
* [ ] Add butterfly evolution animations for milestones
* [ ] Improve mobile responsiveness

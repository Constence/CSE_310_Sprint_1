# Booked - Easy Windows Version 🐾📚

This version avoids WSL. Use it directly in Windows with MSYS2/MinGW.

## Super simple way

1. Open the folder in VS Code.
2. Go to:

```text
backend/run_backend_windows.bat
```

3. Right-click it and choose **Open in Integrated Terminal**, or double-click it in File Explorer.
4. When you see:

```text
Booked backend is running at http://localhost:8080
```

5. Open:

```text
frontend/index.html
```

in your browser.

## If the .bat file does not work

Open the VS Code terminal in the `backend` folder and run:

```bash
g++ -std=c++17 main.cpp -o booked_server.exe -lws2_32
./booked_server.exe
```

## What this project includes

- cute green cat-themed website called **Booked**
- C++ backend
- book storage using `std::map`
- saves book data in `backend/data/books.tsv`
- tracks title, author, rating, and month read
- graph showing reading results across the year
- recommendation assistant

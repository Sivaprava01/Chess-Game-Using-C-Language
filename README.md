# **SDL Chess Project**
Welcome to the SDL Chess Project, a fully functional chess game built in C using SDL2. This project delivers an interactive chess experience with complete chess rules, including special moves like castling and en passant, robust move validation, undo functionality, and a graphical interface. Developed as a college project, it showcases precise move validation and seamless SDL integration.

## **Table of Contents**

1. Features
2. Installation
3. Usage
4. Contributing
5. Author 


## **Features**

- Complete Chess Rules:
  - All standard moves, including pawn promotion and en passant.
  - Castling (king-side and queen-side).
  - Check and checkmate detection.
- Move Validation: Ensures only legal moves, with highlighted valid squares.
- Graphical Interface: SDL2 and SDL_image for smooth board and piece rendering.
- Undo Functionality: Revert moves using a move history stack.
- Move Suggestions: Highlights valid moves for selected pieces.
- Cross-Platform: Runs on Windows, Linux, and macOS.


## **Prerequisites**
- SDL2: For graphics and input.
- SDL_image: For loading piece PNGs.
- C Compiler: GCC or compatible.
- Make: For building with Makefile.

**Install dependencies**


**Windows:**
- Download SDL2 and SDL_image from SDL Website.
- Place in appropriate include/lib folders and link in your IDE/compiler.

## **Build Instructions**

Clone the repository:
```
git clone [YOUR_REPOSITORY_URL]
cd sdl-chess-project
```

Build the project:
```
make
```

Run the game:
```
./chess
```

Usage

- Launch: `./chess`
- Gameplay:
  - Click to select a piece; valid moves highlight in yellow.
  - Click a highlighted square to move.
  - Use Undo to revert moves.
  - Select a piece for pawn promotion when prompted.
  - “Check” or “Checkmate” displays as needed.
- Exit: Close window or press Escape.



## **Contributing**

1. Fork the repository.
2. Create a branch: `git checkout -b feature/your-feature`
3. Commit your changes: `git commit -m "Add feature"`
4. Push to the branch: `git push origin feature/your-feature`
5. Open a Pull Request.


## **Author**

- Your Name - Siva Prava
Contact: sivaprava2006@gmail.com

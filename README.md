# SkyManager

SkyManager is a console-based airline management system developed in C.  
The application allows administrators to manage flights, passengers, and reservations through an interactive terminal interface.

The project was built to practice data structures, dynamic memory management, linked lists, and modular programming in C.

---

# Features

## Flight Management
- Add new flights
- Display all flights
- Search flights by ID
- Modify flight information
- Delete flights

## Passenger Management
- Add passengers
- Display passenger list
- Search passengers by ID
- Modify passenger information
- Delete passengers and related reservations

## Reservation Management
- Create reservations
- Validate seat availability
- Display reservations
- Cancel reservations
- Automatic seat restoration after cancellation

---

# Technical Features

- Dynamic memory allocation using `malloc()` and `free()`
- Singly linked lists
- Input validation system
- Date validation (`YYYY-MM-DD`)
- Time validation (`HH:MM`)
- Automatic ID generation
- Sorted insertion for flights and reservations
- Memory cleanup before program termination

---

# Technologies

- C Programming Language
- Standard C Libraries
  - `stdio.h`
  - `stdlib.h`
  - `string.h`
  - `ctype.h`

---

# Data Structures

The project is based on custom structures:

- `vol`
- `passager`
- `reservation`

Each entity is stored inside linked lists:
- `noeud_v`
- `noeud_p`
- `noeud_r`

---

# Project Structure

```bash
SkyManager/
│
├── main.c
├── README.md
```

---

# Compilation

Using GCC:

```bash
gcc main.c -o skymanager
```

---

# Execution

```bash
./skymanager
```

---

# Example Menu

```text
============================================================
  SkyManager  |  MENU PRINCIPAL
============================================================
  [1] Gestion des Vols
  [2] Gestion des Passagers
  [3] Gestion des Reservations
------------------------------------------------------------
  [0] Quitter
============================================================
```

---

# Validation System

The application includes several validation mechanisms:

- Integer input validation
- Empty string prevention
- Time format validation
- Date format validation
- Seat availability checking
- Linked entity verification

---

# Memory Management

SkyManager uses dynamic memory allocation for all entities.  
All allocated memory is released before program termination to prevent memory leaks.

---

# Learning Objectives

This project was developed to strengthen knowledge in:

- Data structures
- Linked lists
- Dynamic memory management
- Procedural programming
- Modular software design
- Console application development

---

# Future Improvements

- File persistence system
- Authentication system
- Graphical interface
- Database integration
- Flight filtering and sorting
- Reservation history
- Statistics dashboard

---

# Author

Redallah Tarkaoui
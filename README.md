# Smart Food Expiry Tracker

Smart Food Expiry Tracker is a desktop pantry application built with C++17, Qt 6 Widgets, and CMake. It helps track food items by expiry date and highlights items that should be used soon. The project targets Windows and Linux desktops where Qt 6 Widgets is available. It has been built and run in Ubuntu on WSL for this project.

## Features

- Add food with a name, category, quantity, storage location, and integer days until expiry. The number is anchored to the current calendar date and advances daily, including while the app is closed.
- View inventory in expiry order; search normalized full names, including case-insensitive matches and duplicates.
- See a dashboard alert when the soonest-expiring item has two or fewer days remaining.
- Use First shows up to five items ordered by urgency.
- Remove a selected item as consumed or wasted and see session statistics.
- View static, local recipe ideas for supported food names. Unknown items receive a low-waste tip.
- Simulate Scan provides built-in sample items for review and editing. It does not access a camera, OCR, barcode reader, or external service.
- Save inventory to a local file at application exit and load it on startup.

Recipes are simple static ideas, not nutritional advice. There is no account, online service, recipe generation, or cross-device sync.

## Prerequisites

- A C++17 compiler (MSVC on Windows, or GCC/Clang on Linux).
- CMake 3.16 or newer.
- Qt 6 Widgets development files (Qt Core is used by the persistence and test targets).

## Build and run

From a Linux shell or WSL terminal with Qt 6 discoverable through `CMAKE_PREFIX_PATH` or `pkg-config`:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/smart_food_expiry_tracker
```

On Windows, configure CMake with the Qt kit directory if Qt is not already on its search path, then build and launch the Release executable:

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/<version>/msvc<version>_64"
cmake --build build --config Release
.\build\Release\smart_food_expiry_tracker.exe
```

## Inventory storage

Inventory is loaded from `inventory.txt` when the app starts and saved on exit. Qt chooses the per-user `QStandardPaths::AppDataLocation`; the full resolved path is displayed in the application’s save-status line. On Windows this is normally below `%APPDATA%`; on Linux it is normally below `~/.local/share/`. The application creates the data directory when it first saves. If the file is absent, the app starts with an empty inventory. If saving fails, the exit dialog offers Retry, Exit without saving, or Cancel.

The current text file starts with `SFE_INVENTORY_V2`. Each subsequent line contains the item ID and quantity followed by quoted ISO expiry date, name, category, and storage-location fields using `std::quoted`. Quoting preserves spaces, quotes, and backslashes. The absolute expiry date is the source of truth; remaining days are recalculated from today's date at startup and on each calendar-day change while the app runs. Existing `SFE_INVENTORY_V1` files are still read: their relative-day values are converted to dates using the file's last-modified date as the best available baseline, then written in V2 format next time the app saves. Invalid or duplicate records are skipped while valid records load, and the UI reports the line numbers that were skipped. An unsupported header is reported as a load error. Writes use `QSaveFile` so the destination is replaced only after a successful write.

Consumed and wasted counts are session-only; they reset when the app closes. Inventory data persists. The file is local plain text and is not encrypted or synchronized.

## Expiry order and data structures

Active inventory is stored in a min-heap implemented with `std::priority_queue`. Negative days mean expired, zero means expiring today, and positive days are days remaining. The smallest days-until-expiry value is first; equal values are ordered by ascending unique ID for deterministic results.

| Operation | Data structure / approach | Time |
| --- | --- | --- |
| Add item | Heap insertion | O(log n) |
| Get soonest item | Heap top | O(1) |
| Display all items in expiry order | Copy heap and pop each entry | O(n log n) |
| Use First | Copy heap and pop up to five entries | O(k log n), k ≤ 5 |
| Exact-name search | Copy heap, pop/scan entries, normalize names | O(n log n) |
| Remove chosen ID | Copy and pop, then rebuild using heap insertions | O(n log n) |
| Load/save | Parse or serialize records and reconstruct ordered heap view | O(n log n) overall |
| Recipe suggestion | Normalize key and look up in `std::unordered_map` | Average O(1), plus name normalization |

## Tests

Tests are a separate lightweight C++ executable and do not launch the GUI. Build and run them with:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

The suite covers heap order and ID ties, heap-copy display, empty inventory, expiry boundaries and calendar-day aging, normalized full-name search, ID removal, consumed/wasted counts, quoted multi-word persistence fields and expiry dates, missing files, malformed and duplicate records, unsupported headers, restored ID allocation, and a simulated save-path failure.

## Known limitations

- Inventory writes happen on application exit; the app must close successfully or the user must choose to exit without saving after a reported error.
- Statistics are not persisted.
- Recipes are a small fixed catalog and match supported normalized names; they are not generated and do not provide nutritional guidance.
- Simulated scanning does not use hardware or recognition software.
- No cloud backup, synchronization, or multi-device access is included.

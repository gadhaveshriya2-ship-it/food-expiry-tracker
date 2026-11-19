cmake_minimum_required(VERSION 3.16)
project(SmartFoodExpiryTracker VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
add_executable(smart_food_expiry_tracker
    src/main.cpp
    src/mainwindow.cpp src/mainwindow.h
    src/fooditem.h
    src/inventory.cpp src/inventory.h
    src/foodtablemodel.cpp src/foodtablemodel.h
    src/usefirsttablemodel.cpp src/usefirsttablemodel.h
    src/sessionstatistics.h
    src/addfooddialog.cpp src/addfooddialog.h
    src/simulatescandialog.cpp src/simulatescandialog.h
    src/recipes.cpp src/recipes.h
    src/inventorystore.cpp src/inventorystore.h
)
target_link_libraries(smart_food_expiry_tracker PRIVATE Qt6::Widgets)

enable_testing()
add_executable(smart_food_expiry_tracker_tests
    tests/test_main.cpp
    src/fooditem.h
    src/inventory.cpp src/inventory.h
    src/sessionstatistics.h
    src/inventorystore.cpp src/inventorystore.h
)
target_include_directories(smart_food_expiry_tracker_tests PRIVATE src)
target_link_libraries(smart_food_expiry_tracker_tests PRIVATE Qt6::Core)
add_test(NAME smart_food_expiry_tracker_tests COMMAND smart_food_expiry_tracker_tests)

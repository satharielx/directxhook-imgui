# DirectX-Hook Injection
Software application that utilizes DirectX hooking and the ImGui library to create an overlay GUI for a game overlay.

The code in the repository is organized into several files:

    main.cpp:
        This file serves as the main entry point of the application.
        It includes the necessary header files and sets up the DirectX hooking and ImGui.
        The WinMain function initializes the application, creates a window, and enters the main loop.
        The main loop handles user input, updates the features of the application, and renders the overlay GUI.

    DirectXHook.cpp and DirectXHook.h:
        These files contain the code for hooking into DirectX functions and redirecting them to custom functions.
        The hooking is accomplished using the Microsoft Detours library.
        The code intercepts DirectX function calls such as Present and EndScene to render the overlay GUI on top of the target application.

    ImGui_ImplDX9.cpp, ImGui_ImplDX9.h, ImGui_ImplWin32.cpp, and ImGui_ImplWin32.h:
        These files provide the implementation of the ImGui library integration with DirectX 9 and the Windows API.
        ImGui is a widely used library for creating graphical user interfaces (GUIs) in games and applications.
        The code initializes ImGui, sets up the rendering context, and handles input for the overlay GUI.

    ImGuiRenderer.cpp and ImGuiRenderer.h:
        These files contain the rendering logic for the ImGui overlay.
        The code handles drawing various elements of the GUI, such as windows, text, buttons, and checkboxes.
        It utilizes the ImGui API to create and manage the GUI elements.

    Features.cpp and Features.h:
        These files define the specific functionality provided by the application.
        The code includes functions for enabling and disabling various features or capabilities that the application offers.
        The features are controlled through the overlay GUI.

The overall purpose of this project is to create a software application that utilizes DirectX hooking and ImGui to provide a specific functionality or capability. The code hooks into a target application that uses DirectX 9, allowing the overlay GUI to be rendered on top of it. The overlay GUI is created using the ImGui library and provides controls for enabling and configuring various features or capabilities of the application. The specific purpose or functionality provided by the application can be determined based on the implementation of the Features module.

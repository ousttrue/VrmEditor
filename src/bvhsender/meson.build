executable(
    'bvhsender',
    [
        'main.cpp',
        'GlfwPlatform.cpp',
        'GuiApp.cpp',
        'BvhPanel.cpp',
        'Animation.cpp',
        'UdpSender.cpp',
        'Payload.cpp',
    ],
    install: true,
    dependencies: [
        libvrm_dep,
        cuber_dep,
        glfw_dep,
        imgui_dep,
        asio_dep,
    ],
)

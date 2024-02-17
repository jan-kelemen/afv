#include <array>
#include <cstdio>
#include <wchar.h>
#include <string_view>

#define NOMINMAX
#include <windows.h>

#define ESC "\x1b"
#define CSI "\x1b["

namespace
{
    [[nodiscard]] bool enable_virtual_terminal_mode()
    {
        // Set output mode to handle virtual terminal sequences
        HANDLE const out{GetStdHandle(STD_OUTPUT_HANDLE)};
        if (out == INVALID_HANDLE_VALUE)
        {
            return false;
        }
        HANDLE const in{GetStdHandle(STD_INPUT_HANDLE)};
        if (in == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        DWORD original_out_mode{0};
        DWORD original_in_mode{0};
        if (!GetConsoleMode(out, &original_out_mode))
        {
            return false;
        }
        if (!GetConsoleMode(in, &original_in_mode))
        {
            return false;
        }

        DWORD const virtual_terminal_output{
            original_out_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING};
        DWORD const virtual_terminal_input{ENABLE_VIRTUAL_TERMINAL_INPUT};
        if (!SetConsoleMode(out,
                virtual_terminal_output | DISABLE_NEWLINE_AUTO_RETURN))
        {
            // we failed to set both modes, try to step down mode gracefully.
            if (!SetConsoleMode(out, virtual_terminal_output))
            {
                // Failed to set any VT mode, can't do anything here.
                return false;
            }
        }

        if (!SetConsoleMode(in, virtual_terminal_input))
        {
            // Failed to set VT input mode, can't do anything here.
            return false;
        }

        return true;
    }
} // namespace

int main(int argc, char** argv)
{
    if (argc == 1)
    {
        return 1;
    }

    FILE* const file = fopen(argv[1], "rb");
    if (file == nullptr)
    {
        return 2;
    }

    fseek(file, 0, SEEK_END); // seek to end of file
    size_t const bytes{static_cast<size_t>(ftell(file))}; // get current file pointer
    fseek(file, 0, SEEK_SET); // seek back to beginning of file

    if (!enable_virtual_terminal_mode())
    {
        return GetLastError();
    }

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        printf("Couldn't get the console handle. Quitting.\n");
        return -1;
    }
    CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo;
    GetConsoleScreenBufferInfo(hOut, &ScreenBufferInfo);
    COORD Size;
    Size.X =
        ScreenBufferInfo.srWindow.Right - ScreenBufferInfo.srWindow.Left + 1;
    Size.Y =
        ScreenBufferInfo.srWindow.Bottom - ScreenBufferInfo.srWindow.Top + 1;

    printf(CSI "?1049h"); // Enter alternate buffer

    printf(CSI "0;0H");
    printf(CSI "2J"); // Clear screen

    std::array<char, 4096> buffer;
    std::size_t remaining{bytes};
  
    while (remaining != 0 && Size.Y > 0)
    {
        auto const number_of_bytes{std::min(remaining, buffer.size())};
        auto const read{fread(buffer.data(), 1, number_of_bytes, file)};
        if (ferror(file) || read != number_of_bytes)
        {
            return 3;
        }
        std::string_view view{buffer.data(), number_of_bytes};

        size_t current{};
        auto offset{view.find('\n')};
        while (offset != std::string_view::npos && Size.Y > 0)
        {
            
            auto s = std::min<size_t>(offset - current, Size.X);
            auto const written{
                fwrite(buffer.data() + current, 1, s, stdout)};
            if (ferror(stdout) || written != s)
            {
                return 4;
            }

            current = offset;
            offset = view.find('\n', current + 1);
            remaining -= offset - current + 1;
            --Size.Y;
        }
    }

    fclose(file);

    _getwch();

    printf(CSI "?1049l"); // Exit alternate buffer
}

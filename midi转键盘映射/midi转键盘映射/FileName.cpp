// main.cpp

#include <iostream>
#include <windows.h>
#include <commdlg.h> // For Common Item Dialog
#include <shobjidl.h> // For IFileDialog
#include <string>
#include <map>

// Initialize COM library for the thread
#pragma comment(lib, "comctl32.lib")

// Global variable to store the selected file path
std::wstring g_selectedFilePath = L"";

// Function to show the open file dialog
bool ShowOpenFileDialog() {
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize COM library." << std::endl;
        return false;
    }

    IFileDialog* pfd = nullptr;
    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));

    if (SUCCEEDED(hr)) {
        // Set filter for MIDI files
        COMDLG_FILTERSPEC rgSpec[] = {
            { L"MIDI Files", L"*.mid;*.midi" },
            { L"All Files", L"*.*" }
        };
        pfd->SetFileTypes(2, rgSpec);

        // Show the dialog
        hr = pfd->Show(NULL); // NULL means no parent window

        if (SUCCEEDED(hr)) {
            IShellItem* psiResult = nullptr;
            hr = pfd->GetResult(&psiResult);

            if (SUCCEEDED(hr)) {
                PWSTR pszFilePath = nullptr;
                hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                if (SUCCEEDED(hr)) {
                    g_selectedFilePath = pszFilePath;
                    CoTaskMemFree(pszFilePath);
                }
                psiResult->Release();
            }
        }
        pfd->Release();
    }

    CoUninitialize();
    return !g_selectedFilePath.empty();
}


// Keyboard mapping
std::map<BYTE, WORD> noteToKeyMap = {
    {60, 'Z'}, // C4
    {62, 'X'}, // D4
    {64, 'C'}, // E4
    {65, 'V'}, // F4
    {67, 'B'}  // G4
    // Add more mappings as needed
};

// --- MIDI Parsing Logic (Same as before) ---
void ProcessMidiFile(const std::wstring& filePath) {
    // Convert wide string to regular string for fopen
    std::string pathStr(filePath.begin(), filePath.end());

    FILE* file;
    errno_t err = fopen_s(&file, pathStr.c_str(), "rb");
    if (err != 0 || !file) {
        std::wcerr << L"Error opening MIDI file: " << filePath << std::endl;
        return;
    }

    char header_chunk_id[4];
    unsigned int header_chunk_size;
    unsigned short format_type, num_tracks, time_division;

    fread(header_chunk_id, 1, 4, file);
    if (strncmp(header_chunk_id, "MThd", 4) != 0) {
        std::wcerr << L"Invalid MIDI file: Missing MThd chunk." << std::endl;
        fclose(file);
        return;
    }
    fread(&header_chunk_size, 4, 1, file);
    fread(&format_type, 2, 1, file);
    fread(&num_tracks, 2, 1, file);
    fread(&time_division, 2, 1, file);

    // Assume ticks per beat (TPB) format for time_division
    int tpb = (time_division > 0) ? time_division : -(time_division & 0x7FFF);
    double tick_time_ms = 500.0 / tpb; // Assuming 120 BPM (500ms per quarter note)

    // Iterate through tracks
    for (int track_idx = 0; track_idx < num_tracks; ++track_idx) {
        char track_chunk_id[4];
        unsigned int track_chunk_size;
        fread(track_chunk_id, 1, 4, file);
        if (strncmp(track_chunk_id, "MTrk", 4) != 0) {
            std::wcerr << L"Invalid track chunk found." << std::endl;
            fclose(file);
            return;
        }
        fread(&track_chunk_size, 4, 1, file);

        long track_start_pos = ftell(file);
        long track_end_pos = track_start_pos + track_chunk_size;

        unsigned long delta_time_ticks = 0;
        BYTE last_status = 0; // For running status

        while (ftell(file) < track_end_pos) {
            // Read Variable Length Quantity (VLQ) for delta time
            delta_time_ticks = 0;
            unsigned char b;
            do {
                fread(&b, 1, 1, file);
                delta_time_ticks = (delta_time_ticks << 7) | (b & 0x7F);
            } while ((b & 0x80) && ftell(file) < track_end_pos);

            double sleep_ms = delta_time_ticks * tick_time_ms;
            Sleep(static_cast<DWORD>(sleep_ms)); // Wait for the specified time

            BYTE event_byte;
            fread(&event_byte, 1, 1, file);

            BYTE status = (event_byte < 0x80) ? last_status : event_byte; // Handle running status
            if (event_byte >= 0x80) last_status = status; // Update last status if it was a full status byte

            if ((status & 0xF0) == 0x90) { // Note On
                BYTE note_num, velocity;
                fread(&note_num, 1, 1, file);
                fread(&velocity, 1, 1, file);

                if (velocity > 0) { // True Note On
                    if (noteToKeyMap.count(note_num)) {
                        WORD virtualKey = noteToKeyMap[note_num];

                        INPUT input[2];
                        ZeroMemory(input, sizeof(INPUT) * 2);

                        input[0].type = INPUT_KEYBOARD;
                        input[0].ki.wVk = virtualKey;
                        input[0].ki.dwFlags = 0; // Key press

                        input[1].type = INPUT_KEYBOARD;
                        input[1].ki.wVk = virtualKey;
                        input[1].ki.dwFlags = KEYEVENTF_KEYUP; // Key release

                        SendInput(2, input, sizeof(INPUT));

                        std::wcout << L"Processed MIDI Note On: " << (int)note_num << L" -> Sent Key: " << (wchar_t)virtualKey << std::endl;
                    }
                }
            }
            else if ((status & 0xF0) == 0x80) { // Note Off
                // Ignore, handled by the press+release cycle on Note On
                BYTE note_num, velocity;
                fread(&note_num, 1, 1, file);
                fread(&velocity, 1, 1, file);
            }
            else {
                // Skip other MIDI messages
                int msg_len = 0;
                switch (status & 0xF0) {
                case 0xC0: case 0xD0: msg_len = 1; break;
                case 0xF0:
                    if (status == 0xFF) { // Meta event
                        unsigned char b_meta;
                        do { fread(&b_meta, 1, 1, file); } while ((b_meta & 0x80));
                        unsigned long len_meta = 0;
                        do { fread(&b_meta, 1, 1, file); len_meta = (len_meta << 7) | (b_meta & 0x7F); } while ((b_meta & 0x80));
                        fseek(file, len_meta, SEEK_CUR);
                    }
                    else {
                        BYTE b_sys;
                        do { fread(&b_sys, 1, 1, file); } while (b_sys != 0xF7);
                    }
                    break;
                default: msg_len = 2; break;
                }
                for (int i = 0; i < msg_len; ++i) {
                    if (ftell(file) < track_end_pos) {
                        fread(&event_byte, 1, 1, file);
                    }
                }
            }
        }
    }

    fclose(file);
    std::wcout << L"\nMIDI file processed successfully!" << std::endl;
}


int main() {
    std::wcout << L"Please select a MIDI file..." << std::endl;

    if (ShowOpenFileDialog()) {
        std::wcout << L"Selected file: " << g_selectedFilePath << std::endl;
        std::wcout << L"Starting playback simulation and key press generation..." << std::endl;
        ProcessMidiFile(g_selectedFilePath);
    }
    else {
        std::wcout << L"No file was selected or an error occurred." << std::endl;
    }

    std::wcout << L"\nPress Enter to exit...";
    std::cin.get(); // Wait for user input before closing

    return 0;
}
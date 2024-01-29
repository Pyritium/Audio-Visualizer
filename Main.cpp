//#define GLFW_INCLUDE_GLU

#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLEW_STATIC

#include <iostream>
#include <fstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <GL/GLU.h>

#include <Windows.h>
#include <ShObjIdl.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <thread>

#include "Definitions.h"

/*PWSTR RequestFile() {
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    
    IFileOpenDialog* pFileOpen;

    // Create the File Open Dialog object
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
   
    if (SUCCEEDED(hr)) {
        // Set file type filters
        COMDLG_FILTERSPEC FileTypes[] = {
            { L"MP3 Files", L"*.mp3" },
            { L"WAV Files", L"*.wav" },
            { L"OGG Files", L"*.ogg" },
            { L"All Files", L"*.*" }
        };
        pFileOpen->SetFileTypes(_countof(FileTypes), FileTypes);

        // Show the File Open Dialog
        hr = pFileOpen->Show(NULL);

        if (SUCCEEDED(hr)) {
            IShellItem* pItem;
            hr = pFileOpen->GetResult(&pItem);

            if (SUCCEEDED(hr)) {
                PWSTR FilePath;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &FilePath);

                if (SUCCEEDED(hr)) {
                    // Use filePath as the selected file path
                    wprintf(L"Selected File: %s\n", FilePath);

                    // Release allocated memory
                    CoTaskMemFree(FilePath);
                    return FilePath;
                }

                // Release the ShellItem
                pItem->Release();
            }
        }

        // Release the File Open Dialog
        pFileOpen->Release();
    }

    // Uninitialize COM library
    CoUninitialize();

    PWSTR FilePath = RequestFile();
}*/

#define REFTIMES_PER_SEC  1000
#define REFTIMES_PER_MILLISEC  100
#define SAMPLE_RATE (44100)

float AudioBuffer[SAMPLE_RATE];

/*GLFWwindow* WINDOW;
bool Running = true;
float SmootheningRate = 0.9f;
float PreviousVolume = 0.0f;*/

/*void CallBack(const void* input, void* output, unsigned long FrameCount)
{
    const float* out = static_cast<const float*>(input);

    float TotalVolume = 0.0f;

    for (unsigned int i = 0; i < FrameCount; i++)
    {
        float SampleValue = out[i];

        // Compute absolute value (or any other metric based on your requirement)
        TotalVolume += std::abs(SampleValue);

        // You can also update other parts of your audio processing here, if needed
    }

    // Calculate average volume for the frame
    float AverageVolume = TotalVolume / (FrameCount);
    float SmoothVolume = (SmootheningRate * PreviousVolume) + ((1.0f - SmootheningRate) * AverageVolume);
    PreviousVolume = SmoothVolume;
    // Ensure averageVolume is between 0 and 0.9 (or any range you choose)

    // OpenGL visualization
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_QUADS);
    glColor3f(0.25 - SmoothVolume, 0.0, SmoothVolume * 2);  // Color based on volume
    glVertex2f(-1.0, -1.0);  // Bottom-left vertex
    glVertex2f(-1.0, 1.0);  // Top-left vertex
    glVertex2f(1.0, 1.0);  // Top-right vertex
    glVertex2f(1.0, -1.0);  // Bottom-right vertex
    glEnd();

    glfwSwapBuffers(WINDOW);
}*/

int ExitOnError(HRESULT& hr)
{
    if (FAILED(hr))
    {
        return -1;
    }
}

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);


HMENU PopupMenu;
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreatePopupMenu(HWND hwnd);
int main()
{
    glfwInit();
    //glewInit();

    //Pa_Initialize();

    //PaStream* stream;
    //PaError err;

    //IAudioClient* AudioClient = NULL;
    HRESULT hr;
    IMMDeviceEnumerator* Enumerator = NULL;
    IMMDevice* Device = NULL;
    IAudioClient* AudioClient = NULL;
    IAudioCaptureClient* CaptureClient = NULL;
    WAVEFORMATEX* PWFX = NULL;
    BYTE* BData;
    UINT32 PacketLength;
    UINT32 FramesAvailable;
    DWORD flags;

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    ExitOnError(hr);

    hr = CoCreateInstance(
        CLSID_MMDeviceEnumerator, NULL,
        CLSCTX_ALL, IID_IMMDeviceEnumerator,
        (void**)&Enumerator);
    ExitOnError(hr);

    hr = Enumerator->GetDefaultAudioEndpoint(
        eRender, eConsole, &Device);
    ExitOnError(hr);

    hr = Device->Activate(
        IID_IAudioClient, CLSCTX_ALL,
        NULL, (void**)&AudioClient);
    ExitOnError(hr);

    hr = AudioClient->GetMixFormat(&PWFX);
    ExitOnError(hr);

    hr = AudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK,
        REFTIMES_PER_SEC,
        0,
        PWFX,
        NULL);
    ExitOnError(hr);

    hr = AudioClient->GetBufferSize(&FramesAvailable);
    ExitOnError(hr);
     
    hr = AudioClient->GetService(
        IID_IAudioCaptureClient,
        (void**)&CaptureClient);
    ExitOnError(hr);

  

    /*
    PaStreamParameters InputParameters;
    InputParameters.device = Pa_GetDefaultOutputDevice();
    InputParameters.channelCount = PWFX->nChannels;                      // Stereo output
    InputParameters.sampleFormat = paFloat32;              // 32-bit floating point format
    InputParameters.suggestedLatency = Pa_GetDeviceInfo(InputParameters.device)->defaultLowOutputLatency;
    InputParameters.hostApiSpecificStreamInfo = nullptr;

    PaDeviceIndex DeviceIndex = Pa_GetDefaultOutputDevice();
    const PaDeviceInfo* DeviceInfo = Pa_GetDeviceInfo(DeviceIndex);
    std::cout << "Using device: " << DeviceInfo->name << std::endl;
    err = Pa_OpenStream(
        &stream,
        &InputParameters,
        nullptr,
        SAMPLE_RATE,
        512,
        paNoFlag,
        PaStreamCallBack,
        nullptr
        );
    */
    /*err = Pa_OpenDefaultStream(
        &stream,
        0,
        0,
        paFloat32,
        PWFX->nSamplesPerSec,
        paFramesPerBufferUnspecified,
        PaStreamCallBack,
        nullptr
    );
    
    err = Pa_StartStream(stream);*/

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    WINDOW = glfwCreateWindow(640, 480, "Visualizer", NULL, NULL);
    
    if (!WINDOW) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(WINDOW);

    REFERENCE_TIME RequestedDuration = REFTIMES_PER_SEC;
    REFERENCE_TIME ActualDuration;

    ActualDuration = (double)REFTIMES_PER_SEC *
    FramesAvailable / PWFX->nSamplesPerSec;


    hr = AudioClient->Start();  // Start recording.
    ExitOnError(hr);


   
    //std::cout << Text << std::endl;
    
    
   


    Callbacks* CBI = Callbacks::GetInstance();
    CBI->Initialize();

    

    
    HWND hwnd = glfwGetWin32Window(WINDOW);

    CreatePopupMenu(hwnd);
    SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WindowProc);

    std::thread t([] {
        MSG msg = { 0 };
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        };
    });

    
    while (!glfwWindowShouldClose(WINDOW)) {
        std::string Text = GetFile("Settings.xml");
        std::string Tag = "Mode";
        std::vector<std::string> all = GetXMLValuesFromKey(Text, Tag);
        Sleep(ActualDuration / REFTIMES_PER_MILLISEC / 2);


        hr = CaptureClient->GetNextPacketSize(&PacketLength);
        ExitOnError(hr);

        if (PacketLength != 0)
        {
            hr = CaptureClient->GetBuffer(
                &BData, 
                &FramesAvailable, 
                &flags, 
                NULL, 
                NULL);
            ExitOnError(hr);

            hr = CaptureClient->ReleaseBuffer(FramesAvailable);
            ExitOnError(hr);

            hr = CaptureClient->GetNextPacketSize(&PacketLength);
            ExitOnError(hr);
            //Pa_WriteStream(stream, BData, FramesAvailable);
            glClear(GL_COLOR_BUFFER_BIT);

            for (std::string& option : all)
            {
                CBI->ExecuteCallback(option, BData, 0, FramesAvailable);
            }
        } 
            

        //Pa_Sleep(100);
        // Render OpenGL graphics here
        //glClear(GL_COLOR_BUFFER_BIT);

        // Swap front and back buffers
        //glfwSwapBuffers(WINDOW);

        // Poll for and process events
        glfwSwapBuffers(WINDOW);
        glfwPollEvents();
    }
    
    t.join();

    Running = false;
    glfwTerminate();
    //err = Pa_Terminate();

    //if (err != paNoError)
        //printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    return 0;
};

std::vector<std::string> Options = {
    "Heartbeat",
    "MonkeyBars",
    "EnergyRing"
};

LPCWSTR ConvertToLPCWSTR(std::string& str) {
    // Get the required buffer size
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

    // Allocate buffer
    wchar_t* WideStr = new wchar_t[size];

    // Convert the string to wide characters
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, WideStr, size);
    //delete WideStr;
    // Return the wide string
    return WideStr;
}

void UpdatePopupMenu()
{
    std::string Text = GetFile("Settings.xml");
    std::string Tag = "Mode";
    std::vector<std::string> all = GetXMLValuesFromKey(Text, Tag);

    int i = 1;
    for (std::string& s : Options)
    {
        CheckMenuItem(PopupMenu, i, MF_UNCHECKED);
        for (std::string& key : all)
        {

            if (key == s)
            {
                CheckMenuItem(PopupMenu, i, MF_CHECKED);
            }

        }
        i++;
    }
}

void CreatePopupMenu(HWND hwnd) {
    // Create the popup menu
    PopupMenu = CreatePopupMenu();
    int i = 1;
    
    for (std::string& s : Options)
    {
        LPCWSTR WideString = ConvertToLPCWSTR(s);
        AppendMenu(PopupMenu, MF_STRING, i, WideString);
        CheckMenuItem(PopupMenu, i, MF_UNCHECKED);
       
        i++;
    }
    UpdatePopupMenu();
    // Associate the popup menu with the main window
    SetMenu(hwnd, PopupMenu);
}

void SwitchSettings(int Mode)
{
    std::string& Option = Options[Mode - 1];

    std::string Text = GetFile("Settings.xml");
    std::string Tag = "Mode";

    WriteXMLValueUsingKey(Text, Tag, Option, "", true);
    UpdatePopupMenu();
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND:
        SwitchSettings(LOWORD(wParam));
        break;
    case WM_CREATE:
        CreatePopupMenu(hwnd);
        break;

    case WM_CONTEXTMENU:
        // Display the popup menu at the cursor position
        TrackPopupMenu(PopupMenu, TPM_RIGHTBUTTON, LOWORD(lParam), HIWORD(lParam), 0, hwnd, NULL);
        break;

    case WM_DESTROY:
        // Clean up resources
        DestroyMenu(PopupMenu);
        PostQuitMessage(0);
        break;

    default:
        
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}


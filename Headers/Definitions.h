#pragma once
#include "XMLParser.h"
#include <unordered_map>
#include <functional>
#include <any>


GLFWwindow* WINDOW;
bool Running = true;
float SmootheningRate = 0.9f;
float PreviousVolumes[4];
#include <random>
#include <ctime>

std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));

float RandomFloat(float min, float max) {
    std::uniform_real_distribution<float> Distribution(min, max);
    return Distribution(rng);
}

class Singleton {
private:
	Singleton(){};

	static Singleton* instance;
public:
	static Singleton* GetInstance() {
		if (instance == nullptr)
		{
			instance = new Singleton();
		};

		return instance;
	};
	Singleton(const Singleton& obj) = delete;
	Singleton& operator=(const Singleton&) = delete;
	~Singleton(){};
    //void* InheritedInstance;
};


void DrawBar(float x, float width, float height, float RandomFactor) {
    glBegin(GL_QUADS);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex2f(x, -1.0f);           // Bottom-left vertex
    glVertex2f(x + width, -1.0f);   // Bottom-right vertex
    glVertex2f(x + width, RandomFactor-1.0f); // Top-right vertex
    glVertex2f(x, RandomFactor-1.0f);         // Top-left vertex
    glEnd();
}



class Callbacks
{
public:
    static Callbacks* GetInstance() {
        if (instance == nullptr)
        {
            instance = new Callbacks();
        };

        return instance;
    };
    Callbacks(const Callbacks& obj) = delete;
    Callbacks& operator=(const Callbacks&) = delete;
    ~Callbacks() {};

    void Heartbeat(const void* input, void* output, unsigned long FrameCount)
    {
        std::string Text = GetFile("Settings.xml");
        std::string Tag = "Setting";
        //std::cout << Text << std::endl;
        std::vector<std::string> C1 = GetXMLValuesFromKey(Text, Tag, " key=\"ColorMin\"");
        std::vector<std::string> C2 = GetXMLValuesFromKey(Text, Tag, " key=\"ColorMax\"");

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
        float SmoothVolume = (SmootheningRate * PreviousVolumes[0]) + ((1.0f - SmootheningRate) * AverageVolume);
        PreviousVolumes[0] = SmoothVolume;
        // Ensure averageVolume is between 0 and 0.9 (or any range you choose)

        // OpenGL visualization
        //glViewport(480, 0, 640 / 2, 480 / 2);
        glBegin(GL_QUADS);
        glColor3f(0.25 - SmoothVolume, 0.0, SmoothVolume * 2);  // Color based on volume
        glVertex2f(-1.0, -1.0);  // Bottom-left vertex
        glVertex2f(-1.0, 1.0);  // Top-left vertex
        glVertex2f(1.0, 1.0);  // Top-right vertex
        glVertex2f(1.0, -1.0);  // Bottom-right vertex
        glEnd();

        //glfwSwapBuffers(WINDOW);
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    float Iterations[20];

    void MonkeyBars(const void* input, void* output, unsigned long FrameCount)
    {
        const float* out = static_cast<const float*>(input);

        float TotalVolume = 0.0f;

        for (unsigned int i = 0; i < 8; i++)
        {
            float SampleValue = out[i];
           
           
            TotalVolume += std::abs(SampleValue);

            // Compute absolute value (or any other metric based on your requirement)

            // You can also update other parts of your audio processing here, if needed
        }

        float AverageVolume = TotalVolume / (FrameCount);
        float SmoothVolume = (SmootheningRate * PreviousVolumes[1]) + ((1.0f - SmootheningRate) * AverageVolume);
        PreviousVolumes[1] = SmoothVolume;
            
        //glClear(GL_COLOR_BUFFER_BIT);

        //glViewport(0, 0, 640 / 2, 480 / 2);
        for (int i = 0; i < 20; ++i) {
           
            float SmoothIteration = (.8 * Iterations[i]) + ((1.0f - .8) * RandomFloat(300, 1200));
            Iterations[i] = SmoothIteration;

            DrawBar(-1.0f + i * (0.1f), 0.1f, 1.0f, SmoothVolume*Iterations[i]);
        }
        //glfwSwapBuffers(WINDOW);
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void EnergyRing(const void* input, void* output, unsigned long FrameCount)
    {

        const float* out = static_cast<const float*>(input);

        float TotalVolume = 0.0f;

        for (unsigned int i = 0; i < 8; i++)
        {
            float SampleValue = out[i];


            TotalVolume += std::abs(SampleValue);

            // Compute absolute value (or any other metric based on your requirement)

            // You can also update other parts of your audio processing here, if needed
        }

        float AverageVolume = TotalVolume / (FrameCount);
        float SmoothVolume = (SmootheningRate * PreviousVolumes[2]) + ((1.0f - SmootheningRate) * AverageVolume);
        PreviousVolumes[2] = SmoothVolume;

        glBegin(GL_LINE_LOOP);
        glColor3f(1,1,1);  // Color based on volume
        for (int z = 1; z < 4; ++z)
        {
            float RF = RandomFloat(50, 120);
            for (int i = 0; i < 100; ++i) {
                float theta = 2.0f * 3.14 * static_cast<float>(i) / static_cast<float>(100);
                float x = SmoothVolume * RF * cos(theta);
                float y = SmoothVolume * RF * sin(theta);
                glVertex2f(x + 0, y + 0);
                RF = RandomFloat(50, 120);
            }
        }
        glEnd();
    }

    void Initialize() {
        //InheritedInstance = static_cast<Callbacks*>(this);
        CallbacksMap["Heartbeat"] = &Callbacks::Heartbeat;
        CallbacksMap["MonkeyBars"] = &Callbacks::MonkeyBars;
        CallbacksMap["Rings"] = &Callbacks::EnergyRing;
    }

    void ExecuteCallback(const std::string& key, const void* input, void* output, unsigned long FrameCount)
    {
        auto it = CallbacksMap.find(key);
        if (it != CallbacksMap.end())
        {
            
            auto Func = std::any_cast<void (Callbacks::*)(const void*, void*, unsigned long)>(it->second);
            (*this.*Func)(input, output, FrameCount);

            //std::any_cast<void (Callbacks::*) (const void*, void*, unsigned long)> (it->second) (input, output, FrameCount);
            //std::any_cast<void (Callbacks::*)(const void*, void*, unsigned long)>(it->second)(input,output,FrameCount);
            //const std::type_info& ST = it->second.type();
            //const std::type_info& TT = typeid(void (*)(const void*, void*, unsigned long));

            //std::cout << bool(ST == TT) << '\n';
            //std::any_cast <void (*) (const void*, void*, unsigned long)> (it->second) (input, output, FrameCount);
        }
        else
        {
            std::cout << "Key not found!\n";
        }
    }

private:
    Callbacks(){};
    // Create an unordered_map with key as a string and value as HeartbeatFunction
    std::unordered_map<std::string, std::any> CallbacksMap;
    static Callbacks* instance;
    //static Callbacks* instance;
};

Callbacks* Callbacks::instance = nullptr;

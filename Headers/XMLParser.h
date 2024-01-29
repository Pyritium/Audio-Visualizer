#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>

std::string GetFile(std::string filename)
{
    std::string buffer;
    char c;

    std::ifstream in(filename);
    if (!in) {
        const char* FileName = "Settings.xml";

        // Create an instance of ofstream
        std::ofstream OutputFile;
        OutputFile.open(FileName);
        OutputFile << "<Settings>\n  <Mode>Heartbeat</Mode>\n </Settings>\n";
            
           
        OutputFile.close();
        
        in = std::ifstream(FileName);
    }
    while (in.get(c)) buffer += c;
    in.close();

    return buffer;
};


/*std::string StripTags(std::string& text)
{
    unsigned int start = 0, pos;

    while (start < text.size())
    {
        start = text.find("<", start);    if (start == std::string::npos) break;
        pos = text.find(">", start);    if (pos == std::string::npos) break;
        text.erase(start, pos - start + 1);
    }
}*/

std::vector<std::string> GetXMLValuesFromKey(const std::string& xml, const std::string& key, const std::string& extra = "") {
    std::string Start = "<" + key + extra + ">";
    std::string End = "</" + key + ">";

    std::vector<std::string> Values; // Vector of per value, so does not include delimited values.
    size_t LastPos = 0;
    while (true)
    {
        size_t StartPos = xml.find(Start, LastPos);

        if (StartPos == LastPos) {
            std::vector<std::string> Tokens; // WILL be all the delimited values!
            for (std::string& i : Values) // Iterate through every gotten value, and then form an entire vector!
            {
                std::istringstream ss(i);
                std::string token;
                
                while (std::getline(ss, token, ',')) {
                    Tokens.push_back(token);
                }
            }

            return Tokens;
        };
        LastPos = StartPos;
        StartPos += Start.length();
        
        size_t EndPos = xml.find(End, StartPos);
            
        std::string XStr = xml.substr(StartPos, EndPos - StartPos);
        Values.push_back(XStr);
    }
}


void WriteXMLValueUsingKey(std::string& xml, const std::string& key, const std::string& value, const std::string& extra = "", bool concat = 0) {
    std::string Start = "<" + key + extra + ">";
    std::string End = "</" + key + ">";
    std::string cval = value;
    size_t Pos = 0;

    std::ofstream op("Settings.xml", std::ios::trunc);

    while (true)
    {
        size_t StartPos = xml.find(Start, Pos);
        if (StartPos != std::string::npos) {
            StartPos += Start.length();
            size_t EndPos = xml.find(End, StartPos);
            std::string SubString = xml.substr(StartPos, EndPos - StartPos);
            xml.erase(StartPos, EndPos - StartPos);
            if (concat)
            {
                int Extra = 0;
                size_t VPos = ((SubString.find("," + value, Pos) != std::string::npos) ? Extra = 1, SubString.find("," + value, Pos) : ((SubString.find(value + ",", Pos) != std::string::npos) ? Extra = 1, (SubString.find(value + ",", Pos)) : (SubString.find(value, Pos))));
                
                if (VPos != std::string::npos)
                {
                    SubString.erase(VPos, cval.length()+Extra);
                    
                    //std::cout << SubString << std::endl;
                    cval = "";
                }
            }
            
            xml.insert(StartPos, (concat ? (SubString.length() > 0 ? (SubString + (cval.length() > 0 ? "," : "")) : ("")) : "") + cval);
            Pos = StartPos + Start.length();
            op << xml;
        }
        else {
            op.close();
            break;
        }
    }
}


// TODO: Write to value function
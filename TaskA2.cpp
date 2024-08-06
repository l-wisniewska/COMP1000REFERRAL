#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <regex>
#include <map>
#include <string>
#include <iomanip>
using namespace std;

class FileHandler {
public:
    FileHandler(const string& fileName) : fileName(fileName) {}

    bool fileExists() {
        ifstream file(fileName);
        return file.is_open();
    }

    string readFileContents() {
        ifstream file(fileName);
        stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

private:
    string fileName;
};

class SearchHandler {
public:
    SearchHandler(const string& searchString, bool useRegex)
        : searchString(searchString), useRegex(useRegex) {}

    void search(const string& fileContents) {
        splitIntoLines(fileContents);

        if (useRegex) {
            searchWithRegex();
        }
        else {
            searchWithString();
        }
    }

    int getSearchHits() const {
        return searchHits;
    }

    int getTotalWords() const {
        return totalWords;
    }

private:
    string searchString;
    bool useRegex;
    vector<string> lines;
    int searchHits = 0;
    int totalWords = 0;

    void splitIntoLines(const string& fileContents) {
        size_t pos = 0;
        string content = fileContents;
        while ((pos = content.find('\n')) != string::npos) {
            lines.push_back(content.substr(0, pos));
            content.erase(0, pos + 1);
        }
        lines.push_back(content);

        for (const auto& line : lines) {
            totalWords += count(line.begin(), line.end(), ' ') + 1;
        }
    }

    void searchWithRegex() {
        regex regexSearch(searchString);
        cout << "Search Results:" << endl;
        for (size_t i = 0; i < lines.size(); i++) {
            auto words_begin = sregex_iterator(lines[i].begin(), lines[i].end(), regexSearch);
            auto words_end = sregex_iterator();
            for (sregex_iterator j = words_begin; j != words_end; ++j) {
                smatch match = *j;
                size_t wordNumber = 1;
                size_t pos = 0;
                while ((pos = lines[i].find(' ', pos)) != string::npos && pos < match.position()) {
                    wordNumber++;
                    pos++;
                }
                cout << "Found on line " << i + 1 << ", word " << wordNumber << endl;
                searchHits++;
            }
        }
    }

    void searchWithString() {
        cout << "Search Results:" << endl;
        for (size_t i = 0; i < lines.size(); i++) {
            size_t pos = lines[i].find(searchString);
            if (pos != string::npos) {
                size_t wordNumber = 1;
                size_t tempPos = 0;
                while ((tempPos = lines[i].find(' ', tempPos)) != string::npos && tempPos < pos) {
                    wordNumber++;
                    tempPos++;
                }
                cout << "Found on line " << i + 1 << ", word " << wordNumber << endl;
                searchHits++;
            }
        }
    }
};

class CommandLineParser {
public:
    CommandLineParser(int argc, char* argv[]) {
        for (int i = 1; i < argc; i++) {
            args.push_back(string(argv[i]));
        }
    }

    bool validateArguments() const {
        if (args.size() < 2) {
            return false;
        }
        return true;
    }

    string getFileName() const {
        return args[0];
    }

    string getSearchString() const {
        if (useRegexFlag()) {
            return args[1];
        }
        return args[1];
    }

    bool useRegexFlag() const {
        for (const auto& arg : args) {
            if (arg == "-regex") {
                return true;
            }
        }
        return false;
    }

private:
    vector<string> args;
};

class ResultHandler {
public:
    static void writeResultsToCSV(const string& fileName, const string& searchString, double percentage) {
        ofstream resultsFile("results.csv", ios::out | ios::app);
        if (!resultsFile) {
            cout << "Error creating results.csv." << endl;
            return;
        }

        resultsFile << fileName << "," << searchString << "," << fixed << setprecision(2) << percentage << "%" << endl;
        resultsFile.close();
    }
};

class Application {
public:
    void run(int argc, char* argv[]) {
        CommandLineParser parser(argc, argv);

        if (!parser.validateArguments()) {
            cout << "Error: Not enough arguments provided." << endl;
            cout << "Usage: TaskA <filename> <search term> [-regex]" << endl;
            return;
        }

        string fileName = parser.getFileName();
        string searchString = parser.getSearchString();
        bool useRegex = parser.useRegexFlag();

        FileHandler fileHandler(fileName);

        if (!fileHandler.fileExists()) {
            cout << "Error: Unable to open file " << fileName << endl;
            return;
        }

        string fileContents = fileHandler.readFileContents();
        cout << "File Contents:" << endl;
        cout << fileContents << endl;

        SearchHandler searchHandler(searchString, useRegex);
        searchHandler.search(fileContents);

        int searchHits = searchHandler.getSearchHits();
        int totalWords = searchHandler.getTotalWords();
        double percentage = static_cast<double>(searchHits) / totalWords * 100;

        cout << "Search hits: " << searchHits << " out of " << totalWords << " words (" << fixed << setprecision(2) << percentage << "%)" << endl;

        ResultHandler::writeResultsToCSV(fileName, searchString, percentage);
    }
};

int main(int argc, char* argv[]) {
    Application app;
    app.run(argc, argv);
    return EXIT_SUCCESS;
}

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>   
#include <unordered_map>
#include <unordered_set>
#include <limits>      
#include <cctype>      

// Struct to represent one movie record
struct Movie {
    int movieId;                     // unique numeric ID for the movie
    std::string title;               // full movie title 
    int year;                         // release year (or -1 if unavailable)
    std::vector<std::string> genres; // list of genres
};

// Utility function: split a string by a delimiter into parts
std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;   // container to hold results
    std::string token;                 // temporary substring storage
    std::stringstream ss(s);           // wrap input string for parsing

    // Extract substrings until no delimiter is found
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);       // add each extracted part
    }
    return tokens; // return the full list of parts
}

// Helper: extract year from a title if present
int extractYear(std::string& title) {
    // If no year, the year variable is set to -1
    int year = -1;

    // Find the position of the last '(' and ')'
    size_t open = title.find_last_of('(');
    size_t close = title.find_last_of(')');

    
    // Check if both parentheses exist and are in the correct order
    if (open != std::string::npos && close != std::string::npos && close > open) {
        std::string yearStr = title.substr(open + 1, close - open - 1);
        try {
            year = std::stoi(yearStr);           // parse year
            title = title.substr(0, open - 1);   // remove " (YYYY)" part
        }
        catch (...) {
            year = -1; // leave as -1 if parsing fails meaning no year
        }
    }
    return year;
}

// Load movies from the CSV dataset file
std::vector<Movie> loadMovies(const std::string& filename) {
    std::vector<Movie> movies;       // container for all loaded movies
    std::ifstream file(filename);    // open file for reading

    if (!file.is_open()) {           // check if file opened successfully
        std::cerr << "Error: Could not open file " << filename << "\n";
        return movies;               // return empty list on failure
    }

    std::string line;
    std::getline(file, line);        // skip the first row (CSV header)

    // Process each remaining line in the file
    while (std::getline(file, line)) {
        std::stringstream ss(line);  // wrap current line in stringstream
        std::string idStr, title, genresStr;

        // First field: movieId
        std::getline(ss, idStr, ',');

        // Handle titles differently depending on quotes
        if (line.find('"') != std::string::npos) {
            // Case 1: title contains commas → it will be wrapped in quotes
            size_t firstQuote = line.find('"');          // find first quote
            size_t lastQuote = line.find_last_of('"');   // find last quote
            title = line.substr(firstQuote + 1, lastQuote - firstQuote - 1); // extract inside quotes

            // Genres appear after the closing quote + 2 characters (skip the quote and comma)
            genresStr = line.substr(lastQuote + 2);
        }
        else { // Normal case: no quotes in title 
            std::getline(ss, title, ','); 
            std::getline(ss, genresStr, ','); 
        } 
        
        // Construct a Movie object for this line
        Movie m;
        m.movieId = std::stoi(idStr);         // convert movieId string into an integer
        m.year = extractYear(title);          // try to pull out year
        m.title = title;                      // save the extracted movie title
        m.genres = split(genresStr, '|');     // split the genre string into individual genres

        movies.push_back(m);                  // add this movie to the vector
    }

    file.close(); // close the CSV file
    return movies; // return the list of all movies
}


// Build a genre → list of movie indices mapping
std::unordered_map<std::string, std::vector<int>> buildGenreIndex(const std::vector<Movie>& movies) {
    std::unordered_map<std::string, std::vector<int>> genreIndex; // map genre name to list of movie indices

    // Loop through all movies
    for (int i = 0; i < movies.size(); i++) {
        // Each movie can belong to multiple genres
        for (const auto& g : movies[i].genres) {
            genreIndex[g].push_back(i); // store index of movie in that genre’s list
        }
    }

    return genreIndex; // return the completed genre index map
}

// Build a movieId → index mapping
std::unordered_map<int, int> buildIdIndex(const std::vector<Movie>& movies) {
    std::unordered_map<int, int> idIndex; // map movieId to index in movies vector

    // Loop through all movies
    for (int i = 0; i < movies.size(); i++) {
        idIndex[movies[i].movieId] = i; // store index for quick direct lookup
    }

    return idIndex; // return the completed ID index map
}

std::string capitalizeWords(const std::string& s) {
    std::string result = s;
    bool capitalizeNext = true;  // capitalize the first letter and letters after spaces

    for (size_t i = 0; i < result.size(); i++) {
        if (std::isspace(result[i]) || s[i] == '-') {
            capitalizeNext = true;  // next letter should be capitalized
        } 
        else if (capitalizeNext) {
            result[i] = std::toupper(result[i]);
            capitalizeNext = false;
        } 
        else {
            result[i] = std::tolower(result[i]); // make sure other letters are lowercase
        }
    }
    return result;
}


// Convert a string to lowercase
std::string toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

// Function used to gather all the unique genres to display to the user
std::vector<std::string> getAllGenres(const std::vector<Movie>& movies) {
    std::unordered_set<std::string> genreSet;
    for (const auto& m : movies) {
        for (const auto& g : m.genres) {
            genreSet.insert(g); // normalize to lowercase
        }
    }
    std::vector<std::string> allGenres(genreSet.begin(), genreSet.end());
    std::sort(allGenres.begin(), allGenres.end()); // sort alphabetically
    return allGenres;
}

int main() {
    std::string filename = "../data/movies.csv";          // dataset file 
    std::vector<Movie> movies = loadMovies(filename); // load all movies into memory
    auto allGenres = getAllGenres(movies);

    std::cout << "Loaded " << movies.size() << " movies.\n";

    // Build indices for quick lookup
    auto genreIndex = buildGenreIndex(movies); // map genres → list of movie indices
    auto idIndex = buildIdIndex(movies);       // map movieId → index in movies vector

    // Convert genreIndex keys to lowercase for case-insensitive lookup
    std::unordered_map<std::string, std::vector<int>> genreIndexLower;
    for (const auto& pair : genreIndex) {
        genreIndexLower[toLower(pair.first)] = pair.second;
    }

    // CLI Loop
    while (true) {
        std::cout << "\nEnter command (search, quit): "; // Prompts user to either search or quit
        std::string command;
        std::cin >> command;

        if (command == "quit") {
            std::cout << "Exiting program.\n";
            break;
        }
        else if (command == "search") {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // clear buffer

            // Ask for ID first (exclusive)
            std::cout << "Enter movie ID (or press Enter to skip): ";
            std::string idInput;
            std::getline(std::cin, idInput);

            // If user enters a valid ID, it will display the movie ID, title, year if avaliable, and all genres of the movie
            if (!idInput.empty()) {
                int movieId = std::stoi(idInput);
                if (idIndex.find(movieId) != idIndex.end()) {
                    int idx = idIndex[movieId];
                    std::cout << movies[idx].movieId << " | " << movies[idx].title;
                    if (movies[idx].year != -1) std::cout << " (" << movies[idx].year << ")";
                    std::cout << " | Genres: ";
                    for (const auto& g : movies[idx].genres) std::cout << g << " ";
                    std::cout << "\n";
                } 
                else {
                    std::cout << "No movie found with ID: " << movieId << "\n";
                }
                continue; // skip other filters since ID search is unique
            }

            // Title filter
            std::cout << "Enter title keyword (or press Enter to skip): ";
            std::string keyword;
            std::getline(std::cin, keyword);
            std::string keywordLower = toLower(keyword); // Gets the lowercase version of the title

            // Genre filter (multiple input)
            // Displays all possible genres to choose from
            std::cout << "\nAvailable genres:\n";
            for (const auto& g : allGenres) {
                std::cout << " - " << capitalizeWords(g) << "\n";
            }
            // Allows users to enter as many genres as they want to filter for
            std::vector<std::string> filterGenres;
            std::cout << "Enter genres (type 'done' when finished, press Enter to skip):\n";
            while (true) {
                std::string g;
                std::getline(std::cin, g);
                g = toLower(g);
                if (g.empty() || g == "done") break;
                filterGenres.push_back(g);
            }

            // Year filter
            // Stores the year if avaliable, if not, yearFilter is kept as -1
            std::cout << "Enter year (or press Enter to skip): ";
            std::string yearInput;
            std::getline(std::cin, yearInput);
            int yearFilter = -1;
            if (!yearInput.empty()) {
                try { yearFilter = std::stoi(yearInput); } 
                catch (...) { yearFilter = -1; }
            }

            // Apply filters
            std::cout << "\nSearch results:\n";
            int count = 0;
            // Loops through each movie
            for (const auto& m : movies) {
                bool matches = true;

                // Title check
                if (!keyword.empty() && toLower(m.title).find(keywordLower) == std::string::npos) {
                    matches = false;
                }

                // Year check
                if (yearFilter != -1 && m.year != yearFilter) {
                    matches = false;
                }

                // Genre check
                // In the current iteration, it checks if the movie it is on matches the genres selected
                if (!filterGenres.empty()) {
                    bool hasAll = true;
                    for (const auto& fg : filterGenres) {
                        bool found = false;
                        for (const auto& mg : m.genres) {
                            if (toLower(mg) == fg) { 
                                found = true; 
                                break; 
                            }
                        }
                        if (!found) { 
                            hasAll = false; 
                            break; 
                        }
                    }
                    if (!hasAll) matches = false;
                }

                // If there is a match, it displays the title, year, and all genres of the movie
                if (matches) {
                    std::cout << m.movieId << " | " << m.title;
                    if (m.year != -1) std::cout << " (" << m.year << ")";
                    std::cout << " | Genres: ";
                    for (const auto& g : m.genres) std::cout << g << " ";
                    std::cout << "\n";

                    if (++count >= 10) break; // limit results to just 10 movies
                }
            }
            if (count == 0) std::cout << "No matches found.\n";
        }
        else {
            std::cout << "Unknown command. Please try again.\n";
        }
    }
    return 0;
}
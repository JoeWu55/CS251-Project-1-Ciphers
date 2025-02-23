#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "include/caesar_dec.h"
#include "include/caesar_enc.h"
#include "include/subst_dec.h"
#include "include/subst_enc.h"
#include "utils.h"

using namespace std;

// Initialize random number generator in .cpp file for ODR reasons
std::mt19937 Random::rng;

const string ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

// Function declarations go at the top of the file so we can call them
// anywhere in our program, such as in main or in other functions.
// Most other function declarations are in the included header
// files.

// When you add a new helper function, make sure to declare it up here!

/**
 * Print instructions for using the program.
 */
void decryptSubstCipherFile(const QuadgramScorer& scorer);
void printMenu();

int main() {
  Random::seed(time(NULL));
  string command;

  cout << "Welcome to Ciphers!" << endl;
  cout << "-------------------" << endl;
  cout << endl;

  vector<string> quadgrams;
  vector<int> counts;
  ifstream file("english_quadgrams.txt");

  string line;
  while (getline(file, line)) {
    int commaPos = line.find(',');
    // split lines by the comma
    string quadgram = line.substr(0, commaPos);
    int count = stoi(line.substr(commaPos + 1));

    quadgrams.push_back(quadgram);
    counts.push_back(count);
  }

  QuadgramScorer scorer(quadgrams, counts);  // construct it

  do {
    printMenu();
    cout << endl << "Enter a command (case does not matter): ";

    // Use getline for all user input to avoid needing to handle
    // input buffer issues relating to using both >> and getline
    getline(cin, command);
    cout << endl;

    if (command == "F" || command == "f") {
      decryptSubstCipherFile(scorer);
    }

    if (command == "S" || command == "s") {
      decryptSubstCipherCommand(scorer);
    }

    if (command == "E" || command == "e") {
      computeEnglishnessCommand(scorer);
    }

    if (command == "A" || command == "a") {
      applyRandSubstCipherCommand();
    }

    if (command == "C" || command == "c") {
      caesarEncryptCommand();
    }

    if (command == "D" || command == "d") {
      vector<string> dict;
      ifstream file("dictionary.txt");
      string word;

      while (file >> word) {
        dict.push_back(word);
      }

      caesarDecryptCommand(dict);
    }

    if (command == "R" || command == "r") {
      string seed_str;
      cout << "Enter a non-negative integer to seed the random number "
              "generator: ";
      getline(cin, seed_str);
      Random::seed(stoi(seed_str));
    }

    cout << endl;

  } while (!(command == "x" || command == "X") && !cin.eof());

  return 0;
}

void printMenu() {
  cout << "Ciphers Menu" << endl;
  cout << "------------" << endl;
  cout << "C - Encrypt with Caesar Cipher" << endl;
  cout << "D - Decrypt Caesar Cipher" << endl;
  cout << "E - Compute English-ness Score" << endl;
  cout << "A - Apply Random Substitution Cipher" << endl;
  cout << "S - Decrypt Substitution Cipher from Console" << endl;
  cout << "F - Decrypt Substitution Cipher from File" << endl;
  cout << "R - Set Random Seed for Testing" << endl;
  cout << "X - Exit Program" << endl;
}

// "#pragma region" and "#pragma endregion" group related functions in this file
// to tell VSCode that these are "foldable". You might have noticed the little
// down arrow next to functions or loops, and that you can click it to collapse
// those bodies. This lets us do the same thing for arbitrary chunks!
#pragma region CaesarEnc

char rot(char c, int amount) {
  int index = ALPHABET.find(c);
  int rotIndex;

  if ((index + amount) > 25) {
    rotIndex = ((index + amount) % 26);
  } else {
    rotIndex = (index + amount);
  }

  return ALPHABET[rotIndex];
}

string rot(const string& line, int amount) {
  string rotString;

  for (int i = 0; i < line.size(); i++) {
    if (isalpha(line[i])) {
      rotString += rot(toupper(line[i]), amount);
    } else if (isspace(line[i])) {
      rotString += ' ';
    }
  }

  return rotString;
}

void caesarEncryptCommand() {
  int shift;
  string befEncrypt;

  getline(cin, befEncrypt);
  cin >> shift;

  string aftEncrypt = rot(befEncrypt, shift);

  cout << aftEncrypt;
}

#pragma endregion CaesarEnc

#pragma region CaesarDec

void rot(vector<string>& strings, int amount) {
  for (string& word : strings) {     // loop through strings with variable word
    for (char& c : word) {           // loop through the word string with char c
      int index = ALPHABET.find(c);  // find c's index in ALPHABET const
      if (index != string::npos) {   // stops when unable to find c in ALPHABET
        c = ALPHABET[((index + amount) % 26)];
      }
    }
  }
}

string clean(const string& s) {
  string cleaned;
  for (char c : s) {
    if (isalpha(c)) {
      cleaned += toupper(c);
    }
  }

  return cleaned;
}
vector<string> splitBySpaces(const string& s) {
  vector<string> words;
  istringstream pull(s);
  string word;
  while (pull >> word) {
    words.push_back(clean(word));
  }
  return words;
}

string joinWithSpaces(const vector<string>& words) {
  string sentence;
  for (int i = 0; i < words.size(); i++) {
    sentence += words[i];
    if (i != words.size() - 1) {
      sentence += " ";
    }
  }
  return sentence;
}

int numWordsIn(const vector<string>& words, const vector<string>& dict) {
  int count = 0;
  for (const string& word : words) {
    if (find(dict.begin(), dict.end(), word) != dict.end()) {
      count++;
    }
  }
  return count;
}

void caesarDecryptCommand(const vector<string>& dict) {
  string userEncrypt;
  getline(cin, userEncrypt);

  vector<string> words = splitBySpaces(userEncrypt);
  bool Decrypt = false;

  for (int i = 0; i < 26; i++) {
    vector<string> rotWords = words;
    rot(rotWords, i);

    int realWords = numWordsIn(rotWords, dict);
    if (realWords > rotWords.size() / 2) {
      cout << joinWithSpaces(rotWords) << endl;
      Decrypt = true;
    }
  }

  if (!Decrypt) {
    cout << "No good decryptions found" << endl;
  }
}

#pragma endregion CaesarDec

#pragma region SubstEnc

string applySubstCipher(const vector<char>& cipher, const string& s) {
  string convertCap;
  for (char c : s) {
    if (isalpha(c)) {
      convertCap += toupper(c);
    } else {
      convertCap += c;
    }
  }

  string subEncrypt;
  for (char c : convertCap) {
    if (isalpha(c)) {
      int index = ALPHABET.find(c);
      if (index != string::npos) {  // stops when unable to find c in ALPHABET
        subEncrypt += cipher[index];
      }
    } else {
      subEncrypt += c;
    }
  }

  return subEncrypt;
}

void applyRandSubstCipherCommand() {
  string input;
  getline(cin, input);

  vector<char> cipher = genRandomSubstCipher();
  string encrypt = applySubstCipher(cipher, input);

  cout << encrypt << endl;
}

#pragma endregion SubstEnc

#pragma region SubstDec

double scoreString(const QuadgramScorer& scorer, const string& s) {
  double score = 0.0;
  for (int i = 0; i <= s.size() - 4; ++i) {
    string quadgram = s.substr(i, 4);  // create the 4 char strings for quadgram
    score += scorer.getScore(quadgram);  // plug in and add up the score
  }
  return score;
}

void computeEnglishnessCommand(const QuadgramScorer& scorer) {
  string gram;
  getline(cin, gram);

  string cleanedText = clean(gram);

  double score = scoreString(scorer, cleanedText);
  cout << score << endl;
}

vector<char> decryptSubstCipher(const QuadgramScorer& scorer,
                                const string& ciphertext) {
  vector<char> bestCipher = genRandomSubstCipher();
  // first score
  string reversed = applySubstCipher(bestCipher, ciphertext);
  string cleanRev = clean(reversed);
  double max = scoreString(scorer, cleanRev);

  int i = 0;
  while (i < 1000) {
    vector<char> newCipher = bestCipher;

    int first = Random::randInt(25);
    int second = Random::randInt(25);
    while (first == second) {  // make it can't swap itself
      second = Random::randInt(25);
    }

    char temp = newCipher[first];
    newCipher[first] = newCipher[second];
    newCipher[second] = temp;

    // Compute score for new cipher
    string newRev = applySubstCipher(newCipher, ciphertext);
    string newCleanRev = clean(newRev);
    double newScore = scoreString(scorer, newCleanRev);

    if (newScore > max) {
      bestCipher = newCipher;
      max = newScore;
      i = 0;
    } else {
      i++;
    }
  }
  return bestCipher;
}

void decryptSubstCipherCommand(const QuadgramScorer& scorer) {
  string cipherText;
  getline(cin, cipherText);

  vector<char> rightCipher;
  double bestScore = -9000.0;

  for (int i = 0; i < 25; i++) {
    vector<char> tempCipher = decryptSubstCipher(scorer, cipherText);

    string tempRev = applySubstCipher(tempCipher, cipherText);
    string tempCleanRev = clean(tempRev);
    double tempScore = scoreString(scorer, tempCleanRev);

    if (tempScore > bestScore) {
      rightCipher = tempCipher;
      bestScore = tempScore;
    }
  }

  string decrypt = applySubstCipher(rightCipher, cipherText);
  cout << decrypt << endl;
}
#pragma endregion SubstDec

void decryptSubstCipherFile(const QuadgramScorer& scorer) {
  string inFileS;
  string outFileS;

  getline(cin, inFileS);
  getline(cin, outFileS);

  ifstream inFile(inFileS);
  string cipherText;
  string line;
  while (getline(inFile, line)) {
    cipherText += line + "\n";
  }

  inFile.close();

  vector<char> rightCipher;
  double bestScore = -900000.0;

  for (int i = 0; i < 25; i++) {
    vector<char> tempCipher = decryptSubstCipher(scorer, cipherText);

    string tempRev = applySubstCipher(tempCipher, cipherText);
    string tempCleanRev = clean(tempRev);
    double tempScore = scoreString(scorer, tempCleanRev);
    if (tempScore > bestScore) {
      rightCipher = tempCipher;
      bestScore = tempScore;
    }
  }

  string decrypt = applySubstCipher(rightCipher, cipherText);
  ofstream outFile(outFileS);
  outFile << decrypt;
  outFile.close();
}

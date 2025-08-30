# Vigenère Cipher Cryptanalysis

This project implements a classical cryptanalysis tool for the Vigenère cipher, written in C++. The program automatically decrypts ciphertext encrypted with an unknown Vigenère key by applying statistical and pattern-based techniques, including Kasiski examination, Index of Coincidence calculations, Mutual Index of Coincidence alignment, and chi-square frequency analysis.

## Features

- Preprocesses ciphertext by cleaning and uppercasing input.
- Automatically estimates key length using repeated substring analysis and fallback voting.
- Confirms key length with Index of Coincidence statistical testing.
- Recovers the Vigenère key using mutual frequency analysis and chi-square scoring.
- Restores original text formatting (spaces, punctuation, letter case).
- Outputs decrypted plaintext and recovered key in JSON format for easy integration.

## Usage

- Provide the encrypted ciphertext in a file named `input.txt`.
- Build and run the C++ program.
- The decrypted plaintext and key will be saved to `output.json`.

## About

This project showcases how classical frequency and pattern analysis can break polyalphabetic ciphers without prior key knowledge, demonstrating foundational cryptanalysis concepts in practice.

---

Feel free to explore, contribute, or reach out with suggestions!


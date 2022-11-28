#include "casem.hpp"

namespace casem {
	#pragma region LexerAux
	void shift_enregister(int enregistered, int *numregister, int maxlength, int &length)
	{
		for (int i = maxlength - 1; i > 0; i--) {
			numregister[i] = numregister[i - 1];
		}

		numregister[0] = enregistered;
		length++;
	}

	int char_to_int(char c)
	{
		return (c >= 'A') ? (c >= 'a') ? (c - 'a' + 10) : (c - 'A' + 10) : (c - '0');
	}

	bool number_literal_outrange(char* text, int length, bool hexa)
	{
		int start = 0;
		while (text[start] == '0') {
			start++;
		}

		length -= start;

		if (hexa) {
			return length > 8;
		}

		else {
			if (length < 10) {
				return false;
			}

			if (length > 10) {
				return true;
			}

			for (int i = start; i < 10; i++) {
				if ("2147483647"[i] < text[i]) {
					return true;
				}

				else if ("2147483647"[i] > text[i]) {
					return false;
				}
			}

			return false;
		}
	}

	int parse_number_literal(char* text, int length, bool hexa)
	{
		int result = 0;
		int magnitude = hexa ? 16 : 10;
		int start = 0;

		while (text[start] == '0') {
			start++;
		}

		if (hexa && ((length - start) > 8)) {
			start = (length - 8);
		}

		for (int i = start; i < length; i++) {
			result *= magnitude;
			result += char_to_int(text[i]);
		}

		return result;
	}

	bool is_numeral_char(char c, bool hexa)
	{
		if (hexa) {
			return (((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
		}

		else {
			return ((c >= '0') && (c <= '9'));
		}
	}
	#pragma endregion

	#pragma region Declaration Semantics

	#pragma endregion
}


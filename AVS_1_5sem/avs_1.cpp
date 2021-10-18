#include <intrin.h>
#include <iostream>

using namespace std;

int getRandomNumber(int min = -120, int max = 120)
{
	return min + rand() % (max - min + 1);
}

int main()
{
	//set default values

	__int16 A[8], B[8], C[8];
	__int32 D[8] ;

	__m128i vec1, vec2, vec3;
	__m256i vec4;

	srand(time(NULL));

	for (int i = 0; i < 8; i++) 
	{
		A[i] = getRandomNumber();
		B[i] = getRandomNumber();
		C[i] = getRandomNumber();
		D[i] = getRandomNumber();	
	}

	cout << "\nF[i] = A[i] - B[i] * C[i] + D[i]\n\n\n";

	//vec1 = _mm_setr_epi16(A[0], A[1], A[2], A[3], A[4], A[5], A[6], A[7]);
	//vec2 = _mm_setr_epi16(B[0], B[1], B[2], B[3], B[4], B[5], B[6], B[7]);
	//vec3 = _mm_setr_epi16(C[0], C[1], C[2], C[3], C[4], C[5], C[6], C[7]);
	//vec4 = _mm_setr_epi16(D[0], D[1], D[2], D[3], D[4], D[5], D[6], D[7]);

	vec1 = *reinterpret_cast<__m128i*>(&(A));
	vec2 = *reinterpret_cast<__m128i*>(&(B));
	vec3 = *reinterpret_cast<__m128i*>(&(C));
	vec4 = *reinterpret_cast<__m256i*>(&(D));

	__m256i vec1_ = _mm256_cvtepi16_epi32(vec1); //sign extend from  __m128i to __m256i
	__m256i vec2_ = _mm256_cvtepi16_epi32(vec2); //sign extend from __m128i to __m256i
	__m256i vec3_ = _mm256_cvtepi16_epi32(vec3); //sign extend from __m128i to __m256i

	__m256i mul_res = _mm256_mullo_epi32(vec2_, vec3_);	  //B[i]*C[i]

	__m256i result = _mm256_sub_epi32(vec1_, mul_res); // A[i] - B[i]*C[i]

	result = _mm256_add_epi32(result, vec4); //  A[i] - B[i]*C[i] + D[i]

	//__int32* answer = new __int32[8];
	//_mm256_storeu_si256(reinterpret_cast<__m256i*>(&(answer[0])), result);  // store 256-bits of integer data from a into	answer

	__int32* answer = reinterpret_cast<__int32*>(&result);
	
	// print results :

	for (int i = 0; i < 8; i++) {
		cout << answer[i] << " ";
	}

	cout << "\n\n";
	
	for (int i = 0; i < 8; i++) {
		cout << A[i] - B[i] * C[i] + D[i] << " ";
	}

	cout << "\n\n";

	return 0;
}
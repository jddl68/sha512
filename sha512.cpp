#include <iostream>
#include <cstdint>
#include <string>
#include <cstring>
#include <vector>
#include <sstream>
#include <iomanip>

#define rightRotate(x, n) ((x>>n)|(x<<((sizeof(x)<<3)-n)))

uint64_t h[] = {0x6a09e667f3bcc908, 0xbb67ae8584caa73b, 0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1, 
           0x510e527fade682d1, 0x9b05688c2b3e6c1f, 0x1f83d9abfb41bd6b, 0x5be0cd19137e2179};

uint64_t k[] = { 0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc, 0x3956c25bf348b538, 
              0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118, 0xd807aa98a3030242, 0x12835b0145706fbe, 
              0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2, 0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 
              0xc19bf174cf692694, 0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65, 
              0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5, 0x983e5152ee66dfab, 
              0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4, 0xc6e00bf33da88fc2, 0xd5a79147930aa725, 
              0x06ca6351e003826f, 0x142929670a0e6e70, 0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 
              0x53380d139d95b3df, 0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b, 
              0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30, 0xd192e819d6ef5218, 
              0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8, 0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 
              0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8, 0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 
              0x682e6ff3d6b2b8a3, 0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec, 
              0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b, 0xca273eceea26619c, 
              0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178, 0x06f067aa72176fba, 0x0a637dc5a2c898a6, 
              0x113f9804bef90dae, 0x1b710b35131c471b, 0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 
              0x431d67c49c100d4c, 0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817};

std::string hash512(std::string msg_param){
    __uint128_t L = msg_param.length() * 8;
    uint64_t K = (1024 - 1 - 128 - L)%1024;
    uint64_t size = L + 1 + K + 128; // L + 1 + K + 128 % 1024 = 0 NOTE: SIZE IS IN BITS AND MULTIPLE OF 1024

    uint64_t * msg = new uint64_t[size/8];
    memset(msg, 0, size/8);
    
    // Preprocess
    memcpy(msg,  msg_param.c_str(), msg_param.length());

    *((unsigned char*)msg + msg_param.length()) |= 0x80; // Append 1 bit

    //append length
    uint64_t lo = __builtin_bswap64(L);
	uint64_t hi = __builtin_bswap64(L>>64);

    memcpy((unsigned char*)msg + size/8 - (8), &lo, sizeof(uint64_t));
    memcpy((unsigned char*)msg + size/8 - (16), &hi, sizeof(uint64_t));

    //Process
    std::vector<uint64_t> w(80);

    for(uint64_t chunk_i = 0; chunk_i < size / 1024; chunk_i++){    
        memcpy(w.data(), (unsigned char*)msg + chunk_i*(1024/8), 1024/8); // copy ith 1024 sized chunk

        //convert to big endian
        for(int i = 0; i < 16; i++){
            w[i] = __builtin_bswap64(w[i]);
        }

        //extend
        for(int i = 16; i < 80; i++){
            uint64_t s0 = rightRotate(w[i-15], 1) ^ rightRotate(w[i-15], 8) ^ (w[i-15] >> 7);
            uint64_t s1 = rightRotate(w[i-2], 19) ^ rightRotate(w[i-2], 61) ^ (w[i-2] >> 6);
            w[i] = w[i-16] + s0 + w[i-7] + s1;
        }

        //initialize to deafult hash value
        uint64_t aH[8] = {0}; // a-0, b-1, c-2, d-3, e-4, f-5, g-6, h-7
        memcpy(aH, h, 8*sizeof(uint64_t));

        //compress
        for(int i = 0; i < 80; i++){
            uint64_t S1 = rightRotate(aH[4], 14) ^ rightRotate(aH[4], 18) ^ rightRotate(aH[4], 41);
            uint64_t temp1 = aH[7] + S1 + ((aH[4] & aH[5]) ^ ((~aH[4]) & aH[6])) + k[i] + w[i];
            uint64_t S0 = rightRotate(aH[0], 28) ^ rightRotate(aH[0], 34) ^ rightRotate(aH[0], 39);
            uint64_t temp2 = S0 + ((aH[0] & aH[1]) ^ (aH[0] & aH[2]) ^ (aH[1] & aH[2]));
    
            aH[7] = aH[6];
            aH[6] = aH[5];
            aH[5] = aH[4];
            aH[4] = aH[3] + temp1;
            aH[3] = aH[2];
            aH[2] = aH[1];
            aH[1] = aH[0];
            aH[0] = temp1 + temp2;
        }

        //Add the compressed chunk to the current hash value:
        h[0] += aH[0];
        h[1] += aH[1];
        h[2] += aH[2];
        h[3] += aH[3];
        h[4] += aH[4];
        h[5] += aH[5];
        h[6] += aH[6];
        h[7] += aH[7];

    }

    // Concat and Convert to String
	std::stringstream ss;
	// setw and setfill prevent truncating leading zeros
	for (int i = 0; i < 8; i++) {
		ss << std::setw(16) << std::setfill('0') << std::hex << h[i];
	}

    delete[] msg;
	return ss.str();

}

int main(void){
    //std::string inputMsg("a");

    std::string inputMsg("");
    for(int i = 0; i < 100000000;i++){
        inputMsg.append("a");
    }

    std::cout << "Input: " << std::endl;
    //std::cout << inputMsg << std::endl;
    std::cout << "Output: " << std::endl;
    std::string output = hash512(inputMsg);
    //std::cout << output << std::endl;

    return 0;
}
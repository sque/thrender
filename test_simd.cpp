#include <simdpp/sse3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <boost/format.hpp>

template<class T>
inline void to_string_i4(const T * v){
	std::cout << (int)v[0] << ", " << (int)v[1] << ", " << (int)v[2] << ", " << (int)v[3] << std::endl;

}
int main() {

	glm::vec4 v1(0,1,2,3), v2, v3, v4;
	std::cout << glm::to_string(v1) << std::endl;
	std::cout << glm::to_string(v2) << std::endl;

	simdpp::float32x4 xmm1;
	simdpp::load(xmm1, &v1.x);
	simdpp::store(&v2.x, xmm1);

	std::cout << glm::to_string(v1) << std::endl;
	std::cout << glm::to_string(v2) << std::endl;
	if (v1 != v2)
		std::cerr << "Error on loading and storing" << std::endl;

	simdpp::float32x4 xmm2 = simdpp::float32x4::set_broadcast(255.0f);
	simdpp::store(&v3.x, xmm2);
	std::cout << glm::to_string(v3) << std::endl;
	if (v3  != glm::vec4(255.0f,255.0f,255.0f,255.0f))
		std::cerr << "Error on set_broardcat" << std::endl;

	auto xmm3 = simdpp::mul(xmm1, xmm2);
	simdpp::store(&v4.x, xmm3);
	std::cout << glm::to_string(v4) << std::endl;
	if (v4  != glm::vec4(0.000000, 255.000000, 510.000000, 765.000000))
		std::cerr << "Error on mul" << std::endl;


	simdpp::int8x16 xmm4 =  simdpp::to_int32x4(xmm3);
	uint8_t v5[16];
	simdpp::store(v5, xmm4);
	to_string_i4(v5+4);

}

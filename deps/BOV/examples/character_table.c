 /*************************************************************************
  * Character Table example program. Shows all the possible characters
  * available using the BOV library, a wrapper around OpenGL and
  * GLFW (www.glfw.org) to draw simple 2D graphics.
  *------------------------------------------------------------------------
  * Copyright (c) 2019-2020 Célestin Marot <marotcelestin@gmail.com>
  *
  * This software is provided 'as-is', without any express or implied
  * warranty. In no event will the authors be held liable for any damages
  * arising from the use of this software.
  *
  * Permission is granted to anyone to use this software for any purpose,
  * including commercial applications, and to alter it and redistribute it
  * freely, subject to the following restrictions:
  *
  * 1. The origin of this software must not be misrepresented; you must not
  *    claim that you wrote the original software. If you use this software
  *    in a product, an acknowledgment in the product documentation would
  *    be appreciated but is not required.
  *
  * 2. Altered source versions must be plainly marked as such, and must not
  *    be misrepresented as being the original software.
  *
  * 3. This notice may not be removed or altered from any source
  *    distribution.
  *
  *************************************************************************/

#include "BOV.h"
#include <math.h>


int main()
{
	bov_window_t* window = bov_window_new(-1, 1, "Character Table"); // maximized window

	bov_text_t* character_table = bov_text_new(
		(GLubyte[]){
		"Here is a complete list of the available characters, with their hexadecimal value\n"
		"\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\b\xa3\n"
		"       \t 01. \x01\t 02. \x02\t 03. \x03\t 04. \x04\t 05. \x05\t 06. \x06\t 07. \\a\t 08. \\b\t 09.\t 0a. \\n\t 0b. \\v\t 0c. \\f\t 0d. \\r\t 0e. \x0e\t 0f. \x0f \n"
		"  10. \x10\t 11. \x11\t 12. \x12\t 13. \x13\t 14. \x14\t 15. \x15\t 16. \x16\t 17. \x17\t 18. \x18\t 19. \x19\t 1a. \x1a\t 1b. \x1b\t 1c. \x1c\t 1d. \x1d\t 1e. \x1e\t 1f. \x1f \n"
		"  20. \x20\t 21. \x21\t 22. \x22\t 23. \x23\t 24. \x24\t 25. \x25\t 26. \x26\t 27. \x27\t 28. \x28\t 29. \x29\t 2a. \x2a\t 2b. \x2b\t 2c. \x2c\t 2d. \x2d\t 2e. \x2e\t 2f. \x2f \n"
		"  30. \x30\t 31. \x31\t 32. \x32\t 33. \x33\t 34. \x34\t 35. \x35\t 36. \x36\t 37. \x37\t 38. \x38\t 39. \x39\t 3a. \x3a\t 3b. \x3b\t 3c. \x3c\t 3d. \x3d\t 3e. \x3e\t 3f. \x3f \n"
		"  40. \x40\t 41. \x41\t 42. \x42\t 43. \x43\t 44. \x44\t 45. \x45\t 46. \x46\t 47. \x47\t 48. \x48\t 49. \x49\t 4a. \x4a\t 4b. \x4b\t 4c. \x4c\t 4d. \x4d\t 4e. \x4e\t 4f. \x4f \n"
		"  50. \x50\t 51. \x51\t 52. \x52\t 53. \x53\t 54. \x54\t 55. \x55\t 56. \x56\t 57. \x57\t 58. \x58\t 59. \x59\t 5a. \x5a\t 5b. \x5b\t 5c. \x5c\t 5d. \x5d\t 5e. \x5e\t 5f. \x5f \n"
		"  60. \x60\t 61. \x61\t 62. \x62\t 63. \x63\t 64. \x64\t 65. \x65\t 66. \x66\t 67. \x67\t 68. \x68\t 69. \x69\t 6a. \x6a\t 6b. \x6b\t 6c. \x6c\t 6d. \x6d\t 6e. \x6e\t 6f. \x6f \n"
		"  70. \x70\t 71. \x71\t 72. \x72\t 73. \x73\t 74. \x74\t 75. \x75\t 76. \x76\t 77. \x77\t 78. \x78\t 79. \x79\t 7a. \x7a\t 7b. \x7b\t 7c. \x7c\t 7d. \x7d\t 7e. \x7e\t 7f. \x7f \n"
		"  80. \x80\t 81. \x81\t 82. \x82\t 83. \x83\t 84. \x84\t 85. \x85\t 86. \x86\t 87. \x87\t 88. \x88\t 89. \x89\t 8a. \x8a\t 8b. \x8b\t 8c. \x8c\t 8d. \x8d\t 8e. \x8e\t 8f. \x8f \n"
		"  90. \x90\t 91. \x91\t 92. \x92\t 93. \x93\t 94. \x94\t 95. \x95\t 96. \x96\t 97. \x97\t 98. \x98\t 99. \x99\t 9a. \x9a\t 9b. \x9b\t 9c. \x9c\t 9d. \x9d\t 9e. \x9e\t 9f. \x9f \n"
		"  a0. \xa0\t a1. \xa1\t a2. \xa2\t a3. \xa3\t a4. \xa4\t a5. \xa5\t a6. \xa6\t a7. \xa7\t a8. \xa8\t a9. \xa9\t aa. \xaa\t ab. \xab\t ac. \xac\t ad. \xad\t ae. \xae\t af. \xaf \n"
		"  b0. \xb0\t b1. \xb1\t b2. \xb2\t b3. \xb3\t b4. \xb4\t b5. \xb5\t b6. \xb6\t b7. \xb7\t b8. \xb8\t b9. \xb9\t ba. \xba\t bb. \xbb\t bc. \xbc\t bd. \xbd\t be. \xbe\t bf. \xbf \n"
		"  c0. \xc0\t c1. \xc1\t c2. esc c3. esc c4. \xc4\t c5. \xc5\t c6. \xc6\t c7. \xc7\t c8. \xc8\t c9. \xc9\t ca. \xca\t cb. \xcb\t cc. \xcc\t cd. \xcd\t ce. \xce\t cf. \xcf \n"
		"  d0. \xd0\t d1. \xd1\t d2. \xd2\t d3. \xd3\t d4. \xd4\t d5. \xd5\t d6. \xd6\t d7. \xd7\t d8. \xd8\t d9. \xd9\t da. \xda\t db. \xdb\t dc. \xdc\t dd. \xdd\t de. \xde\t df. \xdf \n"
		"  e0. \xe0\t e1. \xe1\t e2. \xe2\t e3. \xe3\t e4. \xe4\t e5. \xe5\t e6. \xe6\t e7. \xe7\t e8. \xe8\t e9. \xe9\t ea. \xea\t eb. \xeb\t ec. \xec\t ed. \xed\t ee. \xee\t ef. \xef \n"
		"  f0. \xf0\t f1. \xf1\t f2. \xf2\t f3. \xf3\t f4. \xf4\t f5. \xf5\t f6. \xf6\t f7. \xf7\t f8. \xf8\t f9. \xf9\t fa. \xfa\t fb. \xfb\t fc. \xfc\t fd. \xfd\t fe. \xfe\t ff. \xff \n"
		"\n---\n\n"
		"\tFor classical ASCII character, the value corresponding to each character is the usual one.\n"
		"It means that you can display \"coucou\", simply by using bov_text_new(\"coucou\", GL_STATIC_DRAW).\n"
		"However, for characters with a value greater or equal to 0xff (128), there are no corresponding ASCII characters...\n"
		"Thus, you will have to use a lesser known feature of C strings: you can insert whatever value you want \n"
		"in a C string by using \\xV, where V is an hexadecimal value\n"
		"For example, to insert the smiley \xa1, you have to use bov_text_new(\"\\xa1\", GL_STATIC_DRAW)\n"
		},
		GL_STATIC_DRAW);

	GLfloat xres = bov_window_get_xres(window);
	GLfloat yres = bov_window_get_yres(window);
	GLfloat minres = xres<yres ? xres : yres;
	GLfloat lineheight = 2.0 / minres * 64.0 / 3.0; // 64/3 pixels in screen coordinates
	bov_text_set_fontsize(character_table, lineheight);
	bov_text_set_pos(character_table, (GLfloat[2]) {-xres / minres + 0.5 * lineheight,
	                                             yres / minres - 1.5 * lineheight});

	while(!bov_window_should_close(window)){
		bov_text_draw(window, character_table);
		bov_window_update_and_wait_events(window);
	}

	printf("Ended correctly - %.2f second\n", bov_window_get_time(window));

	bov_text_delete(character_table);
	bov_window_delete(window);

	return EXIT_SUCCESS;
}

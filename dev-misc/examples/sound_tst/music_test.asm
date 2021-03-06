music_test:
	PMMUSIC_PATTERN music_testPatt

music_testPatt:

	; Note test
	PMMUSIC_WRITERAM (7 * 12 + 11), $30
	PMMUSIC_ROW	N_C_5, $80,  3, 12
	PMMUSIC_ROW	N_CS5, $80, -1, 12
	PMMUSIC_ROW	N_D_5, $80, -1, 12
	PMMUSIC_ROW	N_DS5, $80, -1, 12
	PMMUSIC_ROW	N_E_5, $80, -1, 12
	PMMUSIC_ROW	N_F_5, $80, -1, 12
	PMMUSIC_ROW	N_FS5, $80, -1, 12
	PMMUSIC_ROW	N_G_5, $80, -1, 12
	PMMUSIC_ROW	N_GS5, $80, -1, 12
	PMMUSIC_ROW	N_A_5, $80, -1, 12
	PMMUSIC_ROW	N_AS5, $80, -1, 12
	PMMUSIC_ROW	N_B_5, $80, -1, 12

	; Pulse width test
	PMMUSIC_WRITERAM (7 * 12 + 11), $31
	PMMUSIC_MARK	0
	PMMUSIC_ROW	N_C_5, $80,  3, 1
	PMMUSIC_ROW	N_C_5, $70, -1, 1
	PMMUSIC_ROW	N_C_5, $60, -1, 1
	PMMUSIC_ROW	N_C_5, $50, -1, 1
	PMMUSIC_ROW	N_C_5, $40, -1, 1
	PMMUSIC_ROW	N_C_5, $30, -1, 1
	PMMUSIC_ROW	N_C_5, $20, -1, 1
	PMMUSIC_ROW	N_C_5, $10, -1, 1
	PMMUSIC_ROW	N_C_5, $00, -1, 1
	PMMUSIC_ROW	N_C_5, $10, -1, 1
	PMMUSIC_ROW	N_C_5, $20, -1, 1
	PMMUSIC_ROW	N_C_5, $30, -1, 1
	PMMUSIC_ROW	N_C_5, $40, -1, 1
	PMMUSIC_ROW	N_C_5, $50, -1, 1
	PMMUSIC_ROW	N_C_5, $60, -1, 1
	PMMUSIC_LOOP	0, 3

	; Volume Test
	PMMUSIC_WRITERAM (7 * 12 + 11), $32
	PMMUSIC_ROW	N_A_5, $80,  3, 48
	PMMUSIC_ROW	N_A_5,  -1,  2, 48
	PMMUSIC_ROW	N_A_5,  -1,  1, 48
	PMMUSIC_ROW	N_A_5,  -1,  0, 48

	; Arpeggio & PMMUSIC_LOOP test
	PMMUSIC_WRITERAM (7 * 12 + 11), $33
	PMMUSIC_MARK	0
	PMMUSIC_ROW	N_C_5, $80,  3, 2
	PMMUSIC_ROW	N_DS5, $80,  3, 2
	PMMUSIC_LOOP	0, 15
	PMMUSIC_MARK	0
	PMMUSIC_ROW	N_C_5, $80,  3, 2
	PMMUSIC_ROW	N_E_5, $80,  3, 2
	PMMUSIC_LOOP	0, 15

	; Speed Test
	PMMUSIC_WRITERAM (7 * 12 + 11), $34
	PMMUSIC_ROW	N_A_5, $40,  3, 12
	PMMUSIC_ROW	N____,  -1,  0, 6
	PMMUSIC_ROW	N_A_5, $40,  3, 11
	PMMUSIC_ROW	N____,  -1,  0, 6
	PMMUSIC_ROW	N_A_5, $40,  3, 10
	PMMUSIC_ROW	N____,  -1,  0, 6
	PMMUSIC_ROW	N_A_5, $40,  3, 9
	PMMUSIC_ROW	N____,  -1,  0, 6
	PMMUSIC_ROW	N_A_5, $40,  3, 8
	PMMUSIC_ROW	N____,  -1,  0, 6
	PMMUSIC_ROW	N_A_5, $40,  3, 7
	PMMUSIC_ROW	N____,  -1,  0, 6
	PMMUSIC_ROW	N_A_5, $40,  3, 6
	PMMUSIC_ROW	N____,  -1,  0, 6
	PMMUSIC_ROW	N_A_5, $40,  3, 5
	PMMUSIC_ROW	N____,  -1,  0, 6
	PMMUSIC_ROW	N_A_5, $40,  3, 4
	PMMUSIC_ROW	N____,  -1,  0, 6
	PMMUSIC_ROW	N_A_5, $40,  3, 3
	PMMUSIC_ROW	N____,  -1,  0, 6
	PMMUSIC_ROW	N_A_5, $40,  3, 2
	PMMUSIC_ROW	N____,  -1,  0, 6
	PMMUSIC_ROW	N_A_5, $40,  3, 1
	PMMUSIC_ROW	N____,  -1,  0, 6

	PMMUSIC_WRITERAM (7 * 12 + 11), $20

	PMMUSIC_END

music_test_end:
	.printf "Music Test size: %i bytes\n", music_test_end - music_test

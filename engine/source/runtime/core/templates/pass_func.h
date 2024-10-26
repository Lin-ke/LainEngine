
#ifndef PASS_FUNC_H
#define PASS_FUNC_H
// 快速包装一层函数的宏

#define PASS0R(m_r, m_name) \
	m_r m_name() { return PASSBASE->m_name(); }
#define PASS0RC(m_r, m_name) \
	m_r m_name() const { return PASSBASE->m_name(); }
#define PASS1R(m_r, m_name, m_type1) \
	m_r m_name(m_type1 arg1) { return PASSBASE->m_name(arg1); }
#define PASS1RC(m_r, m_name, m_type1) \
	m_r m_name(m_type1 arg1) const { return PASSBASE->m_name(arg1); }
#define PASS2R(m_r, m_name, m_type1, m_type2) \
	m_r m_name(m_type1 arg1, m_type2 arg2) { return PASSBASE->m_name(arg1, arg2); }
#define PASS2RC(m_r, m_name, m_type1, m_type2) \
	m_r m_name(m_type1 arg1, m_type2 arg2) const { return PASSBASE->m_name(arg1, arg2); }
#define PASS3R(m_r, m_name, m_type1, m_type2, m_type3) \
	m_r m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3) { return PASSBASE->m_name(arg1, arg2, arg3); }
#define PASS3RC(m_r, m_name, m_type1, m_type2, m_type3) \
	m_r m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3) const { return PASSBASE->m_name(arg1, arg2, arg3); }
#define PASS4R(m_r, m_name, m_type1, m_type2, m_type3, m_type4) \
	m_r m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4) { return PASSBASE->m_name(arg1, arg2, arg3, arg4); }
#define PASS4RC(m_r, m_name, m_type1, m_type2, m_type3, m_type4) \
	m_r m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4) const { return PASSBASE->m_name(arg1, arg2, arg3, arg4); }
#define PASS5R(m_r, m_name, m_type1, m_type2, m_type3, m_type4, m_type5) \
	m_r m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5) { return PASSBASE->m_name(arg1, arg2, arg3, arg4, arg5); }
#define PASS5RC(m_r, m_name, m_type1, m_type2, m_type3, m_type4, m_type5) \
	m_r m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5) const { return PASSBASE->m_name(arg1, arg2, arg3, arg4, arg5); }
#define PASS6R(m_r, m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6) \
	m_r m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6) { return PASSBASE->m_name(arg1, arg2, arg3, arg4, arg5, arg6); }
#define PASS6RC(m_r, m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6) \
	m_r m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6) const { return PASSBASE->m_name(arg1, arg2, arg3, arg4, arg5, arg6); }

#define PASS0(m_name) \
	void m_name() { PASSBASE->m_name(); }
#define PASS1(m_name, m_type1) \
	void m_name(m_type1 arg1) { PASSBASE->m_name(arg1); }
#define PASS1C(m_name, m_type1) \
	void m_name(m_type1 arg1) const { PASSBASE->m_name(arg1); }
#define PASS2(m_name, m_type1, m_type2) \
	void m_name(m_type1 arg1, m_type2 arg2) { PASSBASE->m_name(arg1, arg2); }
#define PASS2C(m_name, m_type1, m_type2) \
	void m_name(m_type1 arg1, m_type2 arg2) const { PASSBASE->m_name(arg1, arg2); }
#define PASS3(m_name, m_type1, m_type2, m_type3) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3) { PASSBASE->m_name(arg1, arg2, arg3); }
#define PASS4(m_name, m_type1, m_type2, m_type3, m_type4) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4) { PASSBASE->m_name(arg1, arg2, arg3, arg4); }
#define PASS5(m_name, m_type1, m_type2, m_type3, m_type4, m_type5) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5) { PASSBASE->m_name(arg1, arg2, arg3, arg4, arg5); }
#define PASS6(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6) { PASSBASE->m_name(arg1, arg2, arg3, arg4, arg5, arg6); }
#define PASS7(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6, m_type7) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6, m_type7 arg7) { PASSBASE->m_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7); }
#define PASS8(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6, m_type7, m_type8) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6, m_type7 arg7, m_type8 arg8) { PASSBASE->m_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8); }
#define PASS9(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6, m_type7, m_type8, m_type9) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6, m_type7 arg7, m_type8 arg8, m_type9 arg9) { PASSBASE->m_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9); }
#define PASS10(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6, m_type7, m_type8, m_type9, m_type10) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6, m_type7 arg7, m_type8 arg8, m_type9 arg9, m_type10 arg10) { PASSBASE->m_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10); }
#define PASS11(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6, m_type7, m_type8, m_type9, m_type10, m_type11) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6, m_type7 arg7, m_type8 arg8, m_type9 arg9, m_type10 arg10, m_type11 arg11) { PASSBASE->m_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11); }
#define PASS12(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6, m_type7, m_type8, m_type9, m_type10, m_type11, m_type12) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6, m_type7 arg7, m_type8 arg8, m_type9 arg9, m_type10 arg10, m_type11 arg11, m_type12 arg12) { PASSBASE->m_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12); }
#define PASS13(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6, m_type7, m_type8, m_type9, m_type10, m_type11, m_type12, m_type13) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6, m_type7 arg7, m_type8 arg8, m_type9 arg9, m_type10 arg10, m_type11 arg11, m_type12 arg12, m_type13 arg13) { PASSBASE->m_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13); }
#define PASS14(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6, m_type7, m_type8, m_type9, m_type10, m_type11, m_type12, m_type13, m_type14) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6, m_type7 arg7, m_type8 arg8, m_type9 arg9, m_type10 arg10, m_type11 arg11, m_type12 arg12, m_type13 arg13, m_type14 arg14) { PASSBASE->m_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14); }
#define PASS15(m_name, m_type1, m_type2, m_type3, m_type4, m_type5, m_type6, m_type7, m_type8, m_type9, m_type10, m_type11, m_type12, m_type13, m_type14, m_type15) \
	void m_name(m_type1 arg1, m_type2 arg2, m_type3 arg3, m_type4 arg4, m_type5 arg5, m_type6 arg6, m_type7 arg7, m_type8 arg8, m_type9 arg9, m_type10 arg10, m_type11 arg11, m_type12 arg12, m_type13 arg13, m_type14 arg14, m_type15 arg15) { PASSBASE->m_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15); }

#endif // PASS_FUNC_H

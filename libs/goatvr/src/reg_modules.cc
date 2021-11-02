namespace goatvr {

void register_mod_anaglyph();
#ifdef USE_MOD_OCULUS
void register_mod_oculus();
#endif
#ifdef USE_MOD_OPENVR
void register_mod_openvr();
#endif
void register_mod_sbs();
void register_mod_stereo();

void register_modules()
{
	register_mod_anaglyph();
#ifdef USE_MOD_OCULUS
	register_mod_oculus();
#endif
#ifdef USE_MODE_OPENVR
	register_mod_openvr();
#endif
	register_mod_sbs();
	register_mod_stereo();
}

} // namespace goat

struct glx_config
{
	_Bool
		debug: 1,
		forward_compatible: 1,
		window: 1,
		pixmap: 1,
		pbuffer: 1;

	int vmajor, vminor;
	int sample_buffers, samples;
};

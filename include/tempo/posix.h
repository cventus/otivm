struct pfclock {
	/* clock in use */
	clockid_t clk_id;

	/* process specific epoc */
	time_t ps_epoch;
};

int init_posix_pfclock(struct pfclock *);

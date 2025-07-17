// sv_auto_cstrafe_min_attempts = 50
// sv_auto_cstrafe_success_threshold = 18
// sv_auto_cstrafe_lower_overlap_pct_threshold = 4
// sv_auto_cstrafe_attempt_window = 100
// sv_auto_cstrafe_upper_overlap_pct_threshold = 6
// sv_auto_cstrafe_sequence_length = 20

// underlaps and overlaps internal array are not used for input detection

#define CS_PLAYER_SPEED_RUN           260.0f
#define CS_PLAYER_SPEED_WALK_MODIFIER 0.52f

void UpdateCStrafeStats(CStrafeStats *ss, CCSPlayerController *controller, CCSUsrMsg_CounterStrafe *message)
{
	if (!controller)
	{
		return;
	}
	if (!message->has_press_to_release_ns())
	{
		return;
	}
	CCSPlayerPawn *pawn = controller->GetPlayerPawn();
	if (!pawn)
	{
		return;
	}

	// Make sure the player is running fast enough.
	if (pawn->m_vecAbsVelocity().Length() < CS_PLAYER_SPEED_WALK_MODIFIER * CS_PLAYER_SPEED_RUN)
	{
		return;
	}

	// Only track overlaps and underlaps within 15 ticks (0.23s)
	float press_to_release_ticks = message->press_to_release_ns() / 1e9 * ENGINE_FIXED_TICK_RATE;
	if (fabs(press_to_release_ticks) >= 15.5)
	{
		return;
	}
	// Only track up to 15 ticks window
	// In reality this is more like 7-8 because friction
	uint32 numTicks = V_roundd(press_to_release_ticks);
	if (numTicks > 15)
	{
		return;
	}
	OverlapState state;
	if (numTicks == 0)
	{
		state = PERFECT;
	}
	else if (press_to_release_ticks > 0)
	{
		state = OVERLAP;
	}
	else
	{
		state = UNDERLAP;
	}
	uint32 attemptWindow = sv_auto_cstrafe_attempt_window.GetInt();
	if (attemptWindow > 1000)
	{
		return;
	}

	ss->ResizeBuffer(attemptWindow);
	// sequences is a cyclic array/buffer
	if (ss->sequences.maxCount > 0)
	{
		ss->sequences.Insert(state);
	}

	if (message->press_to_release_ns() >= 0)
	{
		ss->overlaps[numTicks]++;
	}
	else
	{
		ss->underlaps[numTicks]++;
	}

	uint32 seqLength = sv_auto_cstrafe_sequence_length.GetInt();
	uint32 numOverlap = 0;
	uint32 numPerfect = 0; // in sequence
	uint32 maxNumPerfectInSequence = 0;
	uint32 numAttempts = 0;

	if (seqLength > 1000)
	{
		return;
	}

	for (uint32 i = 0; i < attemptWindow; i++)
	{
		// Compute the number of overlap / attempt / perfect in a sequence
		if (i < ss->buffer.sequences.Count())
		{
			OverlapState st = ss->buffer.GetOffsetFromCurrent(i);
			if (st)
			{
				numAttempts++;
				if (st == OVERLAP)
				{
					numOverlap++;
				}
				else if (st == PERFECT)
				{
					numPerfect++;
				}
			}
		}
		// The sequence moves forward, verify if the entry that is out of the sequence is perfect or not
		if (i >= seqLength)
		{
			if (i - seqLength < ss->buffer.sequences.Count())
			{
				OverlapState st = ss->buffer.GetOffsetFromCurrent(i - seqLength);
				if (st == PERFECT)
				{
					numPerfect--;
				}
			}
		}
		// Keep track of the max value in the window
		if (maxNumPerfectInSequence < numPerfect)
		{
			maxNumPerfectInSequence = numPerfect;
		}
	}
	float overlapThreshold = 0.0;
	uint32 minAttempts = sv_auto_cstrafe_min_attempts.GetInt();
	if (numAttempts >= minAttempts)
	{
		uint32 successThreshold = sv_auto_cstrafe_success_threshold.GetInt();
		if (maxNumPerfectInSequence >= successThreshold) // maxNumPerfectInSequence < seqLength
		{
			float ratio = 0.0;
			if (seqLength > successThreshold)
			{
				ratio = (maxNumPerfectInSequence - successThreshold) / (seqLength - successThreshold);
			}
			// Interp the threshold.
			overlapThreshold =
				Lerp(sv_auto_cstrafe_lower_overlap_pct_threshold.GetFloat(), sv_auto_cstrafe_upper_overlap_pct_threshold.GetFloat(), ratio);
		}
	}
	float currentOverlapRate = (float)numOverlap / numAttempts * 100.0;
	// Overlapping too little, kicking?
	if (currentOverlapRate < overlapThreshold)
	{
		ss->ResizeBuffer(0);
		if (sv_auto_cstrafe_kick.GetBool())
		{
			// Kicked for input automation
		}
	}
}

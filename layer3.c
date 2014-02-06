/*
 * layer3.c
 *
 *  Created on: Jan 31, 2014
 *      Author: nathan
 *
 *  PROTOCOL:
 *
 *  ([SYNC][DATA]...)
 *
 *  Messages are sent in chunks. Each chunk has 1 sync byte and up to 15 data
 *  bytes. For the last COUNTDOWN_LENGTH chunks, this is a number counting down
 *  to 0, for all others it is -1. These bytes delimit chunks from each other.
 *  Message length is based on the number of data bytes sent over the course of
 *  the message.
 *
 *  Readers use the following protocol to detect the sync bytes:
 *
 *  Each reader has 2 sync trackers, each of which represent 1 "believed" sync.
 *  Each tracker has a confidence; a tracker with a confidence of 0 doesn't
 *  represent a sync level. Confidence can't drop below 0.
 *
 *  Each presync byte decreases both confidences by 1. If a sync byte matches
 *  one of the trackers, its confidence goes up by 1 and the other's down by 1.
 *  A sync byte automatically matches a tracker with 0 confidence, though only
 *  1 tracker will be matched. If a sync byte comes in that doesn't match either
 *  (they both have confidence > 0) it is an unrecoverable sync error.
 *
 *  Each trackers believed sync is reduced by 1 on each new chunk, so that the
 *  new chunks's sync byte will continue to match. If a tracker's believed sync
 *  is 0, that is assumed to be the last chunk.
 *
 *  If at any time both trackers have a confidence, or there's an error reading
 *  from layer 2, the message is assumed to be corrupted, and -1 is returned.
 *  However, the reading will continue until the whole message is read, to
 *  attempt to stay in sync for future messages.
 *
 */

#include <stdbool.h>
#include <string.h>
#include "layers.h"

////////////////////////////////////////////////////////////////////////////////
// WRITING
////////////////////////////////////////////////////////////////////////////////

//Number of chunks which will have sync bytes.
const static char COUNTDOWN_LENGTH = 100;

//Byte to write before countdown begins. Must be outside [0, COUNTDOWN_LENGTH]
const static char PRESYNC = -1;

//Max number of data bytes per chunk
const static int DATA_PER_CHUNK = MAX_CHUNK_SIZE - 1;

//Index of the sync byte
const static int SYNC_BYTE = 0;

//First index of the data bytes
const static int BEGIN_DATA = 1;

//Max number of consecutive layer2 fails before a permafail
const static int MAX_LAYER2_RETRIES = 10;

//Produce countdown bytes from chunk indexes
inline static char countdown(int chunk)
{
	return chunk > COUNTDOWN_LENGTH ? PRESYNC : chunk;
}

//Attempt up to MAX_LAYER2_RETRIES times to write the chunk.
static inline int attempt_layer2_write(char* buffer, int len)
{
	int result;
	for(int attempt = 0; attempt < MAX_LAYER2_RETRIES; ++attempt)
		if((result = layer2_write(buffer, len)) != -1)
			break;
	return result;
}

inline static int min(int a, int b) { return a < b ? a : b; }

int layer3_write(char* msg, int len)
{
	//Global error handling
	INIT_ERROR();

	/*
	 * The total number of chunks to write. Add some extra ones to A) account
	 * for rounding errors and B) make sure more than 1 or 2 syncs get sent
	 */
	int chunks_remaining = (len / DATA_PER_CHUNK) + 5;

	//Total number of data bytes to write
	int bytes_remaining = len;

	//Moving pointer from which to write data
	const char* seek_data = msg;

	//Temp buffer to store chunks
	char buffer[MAX_CHUNK_SIZE];

	for(; chunks_remaining >= 0; --chunks_remaining)
	{
		//Number of data bytes for this chunk
		int data_bytes_to_write = min(bytes_remaining, DATA_PER_CHUNK);

		//Get countdown byte
		buffer[SYNC_BYTE] = countdown(chunks_remaining);

		//Get data bytes
		memcpy(buffer + BEGIN_DATA, seek_data, data_bytes_to_write);

		//Write to layer 2
		CHECK_ERROR(attempt_layer2_write(buffer, data_bytes_to_write+1) == -1);

		//Update seek and bytes remaining
		seek_data += data_bytes_to_write;
		bytes_remaining -= data_bytes_to_write;
	}

	return len;
}

////////////////////////////////////////////////////////////////////////////////
// READING
////////////////////////////////////////////////////////////////////////////////

//Amount to increase confidence on a matching sync byte
const static int SYNC_CONF_INCREASE = 1;

//Amount to decrease confidence on a mismatching sync byte
const static int SYNC_CONF_DECREASE = -1;

//Amount to decrease confidence of a presync byte
const static int PRESYNC_CONF_DECREASE = -1;

static inline int max(int a, int b) { return a > b ? a : b; }

//Sync tracker
typedef struct {
	char sync;
	int confidence;
} sync_tracker;

//swap 2 trackers. Used to keep sync1 as the higher confidence
static inline void swap(sync_tracker* sync1, sync_tracker* sync2)
{
	sync_tracker tmp = *sync1;
	*sync1 = *sync2;
	*sync2 = tmp;
}

//check for a match. If confidence = 0, set the byte, otherwise compare.
static inline bool match(sync_tracker* sync, char sync_byte)
{
	if(sync->confidence > 0 && sync->sync == sync_byte)
		return true;
	else if(sync->confidence == 0)
	{
		sync->sync = sync_byte;
		return true;
	}
	else
		return false;
}

//Increase or decrease the confidence by amount, keeping >=0
static inline void adjust_confidence(sync_tracker* sync, int amount)
{
	sync->confidence += amount;
	if(sync->confidence < 0)
	{
		sync->confidence = 0;
		sync->sync = PRESYNC;
	}
}

//Update countdown byte
static inline void tick(sync_tracker* sync)
{
	if(sync->sync != PRESYNC)
		--sync->sync;
}

int layer3_read(char* msg, int max)
{
	//Global error handling
	INIT_ERROR();

	//Sync trackers
	sync_tracker sync1 = {PRESYNC, 0};
	sync_tracker sync2 = {PRESYNC, 0};

	int total_data_bytes_read = 0;

	//Set to true if the message is corrupted but sync isn't unrecoverable
	bool message_dead = false;
	char* msg_seek = msg;

	//Keep reading chunks until sync is done
	while(true)
	{
		//Number of bytes read this chunk
		int bytes_read;

		//Buffer to read bytes into
		char read_buffer[MAX_CHUNK_SIZE];

		int attempt = 0;
		do
		{
			//Perform read
			bytes_read = layer2_read(read_buffer, MAX_CHUNK_SIZE);

			//On an error, kill the message and try again, for sync
			if(bytes_read == -1)
			{
				message_dead = true;

				//Fail after MAX_LAYER2_RETRIES retries
				CHECK_ERROR((++attempt) == MAX_LAYER2_RETRIES);
			}
		//Keep retrying until data is read
		} while(bytes_read == -1);

		//Number of message data bytes read
		int data_bytes_read = bytes_read - 1;

		//Update total
		total_data_bytes_read += data_bytes_read;

		//If we go over the max, kill the message
		if(total_data_bytes_read > max)
			message_dead = true;

		//Otherwise write the client buffer.
		else
		{
			memcpy(msg_seek, read_buffer+BEGIN_DATA, data_bytes_read);
			msg_seek += data_bytes_read;
		}

		////////////////////////////////////
		// SYNC ALGORITHM
		////////////////////////////////////

		//Get sync byte
		char sync_byte = read_buffer[0];

		//If it's a presync, decrease all confidence
		if(sync_byte == PRESYNC)
		{
			adjust_confidence(&sync1, PRESYNC_CONF_DECREASE);
			adjust_confidence(&sync2, PRESYNC_CONF_DECREASE);
		}
		//If it matches one, bump that one and decrease the other
		else if(match(&sync1, sync_byte))
		{
			adjust_confidence(&sync1, SYNC_CONF_INCREASE);
			adjust_confidence(&sync2, SYNC_CONF_DECREASE);
		}
		else if(match(&sync2, sync_byte))
		{
			adjust_confidence(&sync2, SYNC_CONF_INCREASE);
			adjust_confidence(&sync1, SYNC_CONF_DECREASE);
		}
		//If it matches neither, unrecoverable sync error
		else
		{
			CHECK_ERROR(true);
		}

		//Keep sync1 the max confidence
		if(sync2.confidence > sync1.confidence)
			swap(&sync1, &sync2);

		//If we're confident this was the last chunk, return
		if(sync1.sync == 0 && sync1.confidence > 0)
			return message_dead ? -1 : total_data_bytes_read;
		else
		{
			tick(&sync1);
			tick(&sync2);
		}
	}
}

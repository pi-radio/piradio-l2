/*
 * config_pl.h
 *
 *  Created on: Jul 30, 2021
 *      Author: george
 */

#ifndef CONFIG_PL_H_
#define CONFIG_PL_H_

#include "utils.h"

void
config_tx(int memfd, uint64_t modulador_ba, uint64_t ifft_dma_ba,
		uint64_t cp_len_addr);

void
config_rx(int memfd, uint64_t sync_word, uint64_t ifft_dma_ba, uint64_t correlator_ba
		, uint16_t sync_word_length);

void
config_packet_gen(int memfd, uint64_t packet_gen_ba, uint64_t dma_pg_ba,
		uint64_t sync_word_addr,uint16_t sync_word_len);

void
config_channel(int memfd, uint64_t fir_ba, uint64_t channel_emulator_ba,
		uint64_t capt_syst_ba, uint16_t freq_offset, uint16_t num_taps,
		uint32_t* taps);

#endif /* CONFIG_PL_H_ */

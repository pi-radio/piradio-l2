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
config_tx(uint64_t modulador_ba, uint64_t ifft_dma_ba, uint64_t cp_len_addr);

void
config_rx(int memfd, uint64_t sync_word, uint64_t ifft_dma_ba,
		uint64_t correlator_ba, uint16_t sync_word_length);

void
config_packet_gen();

#endif /* CONFIG_PL_H_ */

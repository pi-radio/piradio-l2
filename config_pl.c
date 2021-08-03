/*
 * config_pl.c
 *
 *  Created on: Jul 30, 2021
 *      Author: george
 */
#include "config_pl.h"

void
config_tx(int memfd, uint64_t modulador_ba, uint64_t ifft_dma_ba,
		uint64_t cp_len_addr){

	void * mod_base;                   // Modulator reset
	void * dma_ifft_tx_base = NULL;     // DMA config TX IFFT

	map_device(&mod_base, memfd, modulador_ba, MAP_SIZE,
			MAP_MASK, 4);
	map_device(&dma_ifft_tx_base, memfd, ifft_dma_ba, MAP_SIZE,
			MAP_MASK, 0);

	write_to_base(mod_base, 0);        // Reset modulator
	reset_tx_dma_engine(dma_ifft_tx_base);
	config_tx_dma(dma_ifft_tx_base,cp_len_addr, 4);
}

void
config_rx(int memfd, uint64_t sync_word, uint64_t ifft_dma_ba, uint64_t correlator_ba,
		uint16_t sync_word_length){
	void * dma_ifft_rx_base;
	void * dma_correlator_base;

	map_device(&dma_ifft_rx_base, memfd, ifft_dma_ba, MAP_SIZE,
			MAP_MASK, 0);
	map_device(&dma_correlator_base, memfd, correlator_ba, MAP_SIZE,
			MAP_MASK, 0);

	reset_tx_dma_engine(dma_correlator_base);
	config_tx_dma(dma_correlator_base, sync_word,
			sync_word_length);
}

void
config_packet_gen(int memfd, uint64_t packet_gen_ba, uint64_t dma_pg_ba,
		uint64_t sync_word_addr,uint16_t sync_word_len){
	void *packet_gen_base;
	void *dma_pack_gen_base = NULL;    // DMA config for sync word (bit level) in packet generator
	map_device(&dma_pack_gen_base, memfd, dma_pg_ba, MAP_SIZE,
			MAP_MASK, 0);
	map_device(&packet_gen_base, memfd, packet_gen_ba, MAP_SIZE,
			MAP_MASK, 4);

	/* Start packet generator */
	write_to_base(packet_gen_base + 0x04, 0);
	write_to_base(packet_gen_base, 1);

	reset_tx_dma_engine(dma_pack_gen_base);
	config_tx_dma(dma_pack_gen_base, sync_word_addr,
			sync_word_len);
}

void
config_channel(int memfd, uint64_t fir_ba, uint64_t channel_emulator_ba,
		uint64_t capt_syst_ba, uint16_t freq_offset, uint16_t num_taps,
		uint32_t* taps){

	void *capt_syst_delay_base;
	void *fir_filt_base;
	void *chann_emu_base;

	map_device(&capt_syst_delay_base, memfd, capt_syst_ba, MAP_SIZE,
			MAP_MASK, 4);
	map_device(&chann_emu_base, memfd, channel_emulator_ba, MAP_SIZE,
			MAP_MASK, 4);
	map_device(&fir_filt_base, memfd, fir_ba, MAP_SIZE,
					MAP_MASK, 0);

	/* Configure FIR filter taps */

	for(int i = 0; i<= num_taps; i++){
		write_to_base(fir_filt_base + (i * 4), taps[i]);
	}

	write_to_base(chann_emu_base, freq_offset);
}

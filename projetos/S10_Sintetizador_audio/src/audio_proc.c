#include "audio_proc.h"

volatile float fir_coeffs[FILTER_ORDER];  // Coeficientes globais
volatile float filter_history[FILTER_ORDER] = {0}; // Mantém estado entre chamadas

// Buffers e variáveis globais para DMA
uint16_t capture_buffer[BUFFER_SIZE];
uint8_t dma_chan;
dma_channel_config dma_cfg;

void setup_leds(){
    // Configura o pino do LED como saída.
    gpio_init(LED_R);
    gpio_init(LED_G);
    gpio_init(LED_B);
    gpio_set_dir(LED_R, GPIO_OUT);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_set_dir(LED_B, GPIO_OUT);
    gpio_put(LED_R, 0); // Garante que o LED comece apagado.
    gpio_put(LED_G, 0);
    gpio_put(LED_B, 0);
}

void led_put_color(bool R, bool G, bool B){
    gpio_put(LED_R, R); // Garante que o LED comece apagado.
    gpio_put(LED_G, G);
    gpio_put(LED_B, B);
}

void micriphone_setup(){
    // Configura o pino do microfone como entrada analógica
    adc_gpio_init(MIC_PIN);

    adc_init();//inicializa hardware ADC
    
    adc_select_input(MIC_CHANEL);// Canal ADC: GPIO28 = Canal 2 (28-26=2)
    adc_fifo_setup(
        true, // Habilitar FIFO
        true, // Habilitar request de dados do DMA
        1, // Threshold para ativar request DMA é 1 leitura do ADC
        false, // Não usar bit de erro
        false // Não fazer downscale das amostras para 8-bits, manter 12-bits.
    );

    adc_set_clkdiv(ADC_CLOCK_DIV);
    adc_set_round_robin(0);

    // 2. Configuração do DMA
    dma_chan = dma_claim_unused_channel(true);
    dma_cfg = dma_channel_get_default_config(dma_chan);

    channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_16); // Tamanho da transferência é 16-bits (usamos uint16_t para armazenar valores do ADC)
    channel_config_set_read_increment(&dma_cfg, false); // Desabilita incremento do ponteiro de leitura (lemos de um único registrador)
    channel_config_set_write_increment(&dma_cfg, true); // Habilita incremento do ponteiro de escrita (escrevemos em um array/buffer)
    channel_config_set_dreq(&dma_cfg, DREQ_ADC); // Usamos a requisição de dados do ADC
}

void mic_get_sample(){
//   printf("Comecei a gravar!\n");
  adc_fifo_drain(); // Limpa o FIFO do ADC.
  adc_run(false); // Desliga o ADC (se estiver ligado) para configurar o DMA.
  absolute_time_t before_time = get_absolute_time();
//   absolute_time_t total_time;

  dma_channel_configure(dma_chan, &dma_cfg,
    capture_buffer, // Escreve no buffer.
    &(adc_hw->fifo), // Lê do ADC.
    BUFFER_SIZE, // Faz SAMPLES amostras.
    false // desLiga o DMA.
  );
//   printf("DMA configurado!\n");
 
   // 2. Configura hardware para controle de taxa
    // adc_set_fifo_enabled(true);
    adc_run(true);  // Inicia conversões contínuas
    
    // 3. Inicia DMA com controle de taxa
    dma_channel_start(dma_chan);

    // 4. Espera conclusão com timeout
    absolute_time_t timeout = make_timeout_time_ms((DURATION_SEC+1)*1000 );  // 6s para 5s de áudio
    while (dma_channel_is_busy(dma_chan)) {
        if (time_reached(timeout)) {
            printf("Erro: Timeout na captura!\n");
            break;
        }
    }
    
    adc_run(false);
    int64_t total_time = absolute_time_diff_us(before_time, get_absolute_time());
    printf("Gravação concluída! Tempo: %lld µs\n", total_time);

}

void setup_buzzer_pwm(){//deve ser configurado assim div_int=2 div_frac=16 e wrap=947(período). Isso para ter uma frequència perto de 44kHz
    uint slice;
    //aqui prepara o Buzzer direito
    gpio_set_function(BUZZER_PIN,GPIO_FUNC_PWM);//configura gpio para pwm
    slice = pwm_gpio_to_slice_num(BUZZER_PIN); //pega o slice associado à gpio
    // pwm_set_clkdiv(slice,BUZ_PWM_INT_DIV);
    pwm_set_clkdiv_int_frac4(slice, BUZ_PWM_INT_DIV, BUZ_PWM_FRAC_DIV);//define os divisores de clk para valores que o aproximam de 44kHz
    pwm_set_wrap(slice,PERIODO);
    pwm_set_gpio_level(BUZZER_PIN,0);
    pwm_set_enabled(slice,true);

//aqui prepara o Buzzer esquerdo
    gpio_set_function(BUZZER_PIN2,GPIO_FUNC_PWM);//configura gpio para pwm
    slice = pwm_gpio_to_slice_num(BUZZER_PIN2); //pega o slice associado à gpio
    // pwm_set_clkdiv(slice,BUZ_PWM_INT_DIV);
    pwm_set_clkdiv_int_frac4(slice, BUZ_PWM_INT_DIV, BUZ_PWM_FRAC_DIV);//define os divisores de clk para valores que o aproximam de 44kHz
    pwm_set_wrap(slice,PERIODO);
    pwm_set_gpio_level(BUZZER_PIN2,0);
    pwm_set_enabled(slice,true);
}

void buzzer_put_sound(){

    // 1. Obtém o slice do PWM associado ao buzzer
    // uint8_t slice = pwm_gpio_to_slice_num(BUZZER_PIN);
    // uint8_t slice2 = pwm_gpio_to_slice_num(BUZZER_PIN2);
    
    // 2. Configura timer para controle de timing
    // absolute_time_t next_time = get_absolute_time();
    // const uint32_t sample_interval_us = 100;  // 1/10.000 seg = 100 µs
    
    // 3. Loop através de todas as amostras
    for (int i = 0; i < BUFFER_SIZE; i++) {
        // 4. Mapeia valor ADC (0-4095) para PWM (0-947)
        uint16_t pwm_value = capture_buffer[i];
        
        // 5. Aplica o valor ao PWM
        pwm_set_gpio_level(BUZZER_PIN, pwm_value);
        pwm_set_gpio_level(BUZZER_PIN2, pwm_value);
        
        // 6. Espera até o próximo intervalo
        // next_time = delayed_by_us(next_time, SAMPLE_INTERVAL_US);
        // busy_wait_until(next_time);
        sleep_us(SAMPLE_INTERVAL_US);
    }
    
    // 7. Desliga o buzzer após reprodução
    pwm_set_gpio_level(BUZZER_PIN, 0);
    pwm_set_gpio_level(BUZZER_PIN2, 0);
}


void generate_blackman_fir_coeffs(float cutoff_freq) {
    const uint8_t M = FILTER_ORDER;
    const float fc = cutoff_freq;  // Frequência de corte normalizada (0.0-1.0)
    float sum = 0.0f;
    
    for (int n = 0; n < M; n++) {
        // Resposta do filtro passa-baixas ideal
        float sinc;
        // if (fabsf(n - M/2.0f) < 1e-4) {
        //     sinc = 2.0f * fc;
        // } else {
        //     float angle = 2 * M_PI * fc * (n - M/2.0f);
        //     sinc = sinf(angle) / (M_PI * (n - M/2.0f)); // Adicionar M_PI
        // }
        if (n == M/2) {
            sinc = 2.0f * fc;  // Evita divisão por zero
        } else {
            float angle = 2 * M_PI * fc * (n - M/2);
            sinc = sinf(angle) / (angle);
        }
        
        // Janela de Blackman
        float window = 0.42f - 0.5f * cosf(2*M_PI*n/(M-1)) 
                     + 0.08f * cosf(4*M_PI*n/(M-1));
        
        // Coeficiente final
        fir_coeffs[n] = sinc * window;
        sum += fir_coeffs[n];
    }
    
    // Normalização para ganho unitário
    for (int n = 0; n < M; n++) {
        fir_coeffs[n] /= sum;
    }
    // printf("Acabei de fazer os coeficientes\n");
}

void apply_fir_filter(uint16_t* buffer, int buffer_size) {
    // Buffer temporário para amostras atuais (economiza memória)
    float input_samples[FILTER_ORDER] = {0};
    
    for (int i = 0; i < buffer_size; i++) {
        // Atualiza janela deslizante
        for (int j = FILTER_ORDER-1; j > 0; j--) {
            input_samples[j] = input_samples[j-1];
        }
        input_samples[0] = (float)buffer[i] / 4095.0f;  // Normaliza para 0.0-1.0
        
        // Aplica convolução
        float output = 0.0f;
        for (int k = 0; k < FILTER_ORDER; k++) {
            output += fir_coeffs[k] * input_samples[k];
        }
        
        // Converte e satura o valor
        output = output * 4095.0f;
        if (output < 0) output = 0;
        if (output > 4095) output = 4095;
        
        buffer[i] = (uint16_t)output;  // Sobrescreve in-place
    }
    // printf("Terminei a conversão do Buffer\n");
}

void apply_fir_filter2(uint16_t* buffer, int buffer_size) {
    for (int i = 0; i < buffer_size; i++) {
        // Atualiza histórico
        for (int j = FILTER_ORDER-1; j > 0; j--) {
            filter_history[j] = filter_history[j-1];
        }
        filter_history[0] = (float)buffer[i];
        
        // Aplica convolução
        float output = 0.0f;
        for (int k = 0; k < FILTER_ORDER; k++) {
            output += fir_coeffs[k] * filter_history[k];
        }
        
        // Saturação sem conversão desnecessária
        if (output < 0) output = 0;
        if (output > 4095) output = 4095;
        
        buffer[i] = (uint16_t)output;
    }
}
void apply_notch_filter(uint16_t* buffer, int buffer_size, float notch_freq, float Q) {
    // Implementação simplificada de filtro IIR notch
    static float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    const float w0 = 2 * M_PI * notch_freq / MIC_SAMPLES_RATE;
    const float alpha = sinf(w0) / (2 * Q);
    
    const float b0 =  1;
    const float b1 = -2 * cosf(w0);
    const float b2 =  1;
    const float a0 =  1 + alpha;
    const float a1 = -2 * cosf(w0);
    const float a2 =  1 - alpha;

    for (int i = 0; i < buffer_size; i++) {
        float x = buffer[i];
        float y = (b0/a0)*x + (b1/a0)*x1 + (b2/a0)*x2
                - (a1/a0)*y1 - (a2/a0)*y2;

        x2 = x1;
        x1 = x;
        y2 = y1;
        y1 = y;
        
        buffer[i] = (uint16_t)(y > 4095 ? 4095 : y < 0 ? 0 : y);
    }
}

void apply_pre_emphasis(uint16_t* buffer, int buffer_size, float alpha) {
    static float prev = 0;
    for (int i = 0; i < buffer_size; i++) {
        float current = buffer[i];
        float emphasized = current - alpha * prev;
        prev = current;
        
        // Normaliza para faixa original
        emphasized = emphasized * (4095.0f / 4095.0f * (1 + fabsf(alpha)));
        buffer[i] = (uint16_t)(emphasized > 4095 ? 4095 : emphasized < 0 ? 0 : emphasized);
    }
}

void filtrar_audio(){
    apply_fir_filter(capture_buffer, BUFFER_SIZE);

    // //filtragem supostamente mais sofisticada
    // apply_pre_emphasis(capture_buffer, BUFFER_SIZE, 0.95f);
    // apply_fir_filter(capture_buffer, BUFFER_SIZE);
    // apply_notch_filter(capture_buffer, BUFFER_SIZE, 60.0f, 30.0f); // 60Hz
}

void normalize_waveform(uint8_t* waves) {
    // 1. Encontrar o valor RMS máximo em todo o buffer para normalização
    float max_rms = 0.01f;  // Evita divisão por zero
    int segment_size = BUFFER_SIZE / DISPLAY_WIDTH;
    
    // Primeira passada: calcular RMS máximo global
    for (int i = 0; i < DISPLAY_WIDTH; i++) {
        int start = i * segment_size;
        int end = (i == DISPLAY_WIDTH - 1) ? BUFFER_SIZE : start + segment_size;
        
        float sum_squares = 0;
        for (int j = start; j < end; j++) {
            // Converter para valor centrado em zero (-2048 a 2048)
            float sample = (float)capture_buffer[j] - 2048;
            sum_squares += sample * sample;
        }
        
        float rms = sqrtf(sum_squares / (end - start));
        if (rms > max_rms) max_rms = rms;
    }

    // 2. Segunda passada: calcular e normalizar cada segmento
    for (int i = 0; i < DISPLAY_WIDTH; i++) {
        int start = i * segment_size;
        int end = (i == DISPLAY_WIDTH - 1) ? BUFFER_SIZE : start + segment_size;
        
        float sum_squares = 0;
        for (int j = start; j < end; j++) {
            float sample = (float)capture_buffer[j] - 2048;
            sum_squares += sample * sample;
        }
        
        float rms = sqrtf(sum_squares / (end - start));
        
        // Normalizar para 0-HEIGHT e converter para inteiro
        float normalized = (rms / max_rms) * (DISPLAY_HEIGHT - 1);
        waves[i] = (uint8_t)(normalized > DISPLAY_HEIGHT ? DISPLAY_HEIGHT : normalized);
    }
}
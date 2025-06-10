#include "oled_control.h"


//declaração global do OLED para usar em funções fora da main
uint8_t ssd[ssd1306_buffer_length];
// Preparar área de renderização para o display (ssd1306_width pixels por ssd1306_n_pages páginas)
struct render_area frame_area = {
    start_column : 0,
    end_column : ssd1306_width - 1,
    start_page : 0,
    end_page : ssd1306_n_pages - 1
};


void setup_OLED(){
    // Inicialização do i2c
    i2c_init(i2c1, 500 * 1000); //i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // Processo de inicialização completo do OLED SSD1306
    ssd1306_init();
    calculate_render_area_buffer_length(&frame_area);
  }

void oled_clear(){
    memset(ssd, 0, ssd1306_buffer_length);
    // render_on_display(ssd, &frame_area);
}

void oled_print_info(){
    char buffer[16];
    sprintf(buffer, "Aperte A=Gravar");
    ssd1306_draw_string(ssd, 0, 0, buffer);
    sprintf(buffer, "Aperte B=Tocar");
    ssd1306_draw_string(ssd, 0, 8, buffer);
}

void oled_render(){
    render_on_display(ssd, &frame_area);
}

void oled_print_msg(char *str){
    char buffer[16];
    sprintf(buffer,"%s",str);
    ssd1306_draw_string(ssd, 0, 0, buffer);
}

void oled_print_wave(uint8_t* waves){
    
    oled_clear();
    for (uint8_t i = 0; i < DISPLAY_WIDTH; i++)
    {   uint8_t defasagem=(DISPLAY_HEIGHT-waves[i])/2; //defasagem para centralizar as linhas verticalmente
        // printf("Wave[%d] = %d \n",i,waves[i]);
        ssd1306_draw_line(ssd,i,defasagem,i,waves[i]+defasagem,true);
    }
    
    // ssd1306_draw_line

}
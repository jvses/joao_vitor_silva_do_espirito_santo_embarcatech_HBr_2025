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

volatile uint16_t hist_array[13]={0};//array do historiaograma com todos elementos inicializado com valor 0
volatile uint16_t amostras={0};
volatile int8_t balls_on_board={-1}; // bolas caindo no tabuleiro(em 0 já tem uma pois funciona como um índice)
volatile bool view_hist=false; //variável que seleciona se vai printar o tabuleiro ou o histograma
volatile Bola balls[50];//crio a struct de bolas simultâneas que podem cair no tabuleiro que podem ser editadas nas funções de callback

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

void oled_print_info(int8_t desvio){
    char buffer[30];
    sprintf(buffer, "Ams %d", amostras);
    ssd1306_draw_string(ssd, 0, 0, buffer);
    sprintf(buffer, "V %d", desvio);
    ssd1306_draw_string(ssd, 78, 0, buffer);
}

void oled_render(){
    render_on_display(ssd, &frame_area);
}

bool biased_random_fixed(int8_t bias_percent) {
    // Bias como porcentagem inteira (0-100)
    uint32_t rand_num = get_rand_32();
    return (rand_num % 100) < bias_percent;
}

bool letf_or_right(int8_t sig){
    // return ( (get_rand_32()%2) ? true : false);
    return biased_random_fixed(sig);
}

void draw_board(){
    int8_t adrx_init={64},rows={11},points_in_row={1},current_row={10};
    // printf("Fiz o draw\n");

    for (int8_t j=rows; j>=0 ; j-- ){
        // printf("Entrei no primeiro loop\n");
        for(uint8_t i=0; i<(points_in_row);i++){
            // printf("Indice_i={%d}  x={%d} e y={%d}\n",i,adrx_init-i*(STEP),current_row);
            ssd1306_set_pixel(ssd,adrx_init+i*(STEP),current_row,true);
            // sleep_ms(200);
        }
        adrx_init-=STEP/2;

        // printf("Novalinha\n");
        current_row+=STEP/2;
        points_in_row+=1;
    }
    // render_on_display(ssd, &frame_area);
    // ssd1306_set_pixel(ssd,);
}

void draw_ball(uint8_t indx){//como passo a struct diretamente eu uso b.(parametro) para mudas os valores
    ssd1306_fill_rect(ssd, balls[indx].oled_addr[0]-1, balls[indx].oled_addr[1]-1, balls[indx].oled_addr[0]+1, balls[indx].oled_addr[1]+1, true);
}

void draw_bias_line(int8_t sigma){
    ssd1306_draw_line(ssd,SIG_CENTRE_X,SIG_CENTRE_Y,SIG_CENTRE_X+100,SIG_CENTRE_Y,true);
    ssd1306_fill_rect(ssd,(SIG_CENTRE_X+sigma-2),SIG_CENTRE_Y-3,(SIG_CENTRE_X+sigma+2),SIG_CENTRE_Y+3,true);
}

void setup_balls(int8_t sig){//como aqui é um ponteiro eu tenho que usar -> para mudar as coisas
    for(uint8_t i=0; i<50;i++){
        balls[i].direction=letf_or_right(sig);
        balls[i].oled_addr[0]=64;
        balls[i].oled_addr[1]=6;
        balls[i].real_addr[0]=6;
        balls[i].real_addr[1]=1;
    }
}

  bool update_falls_callback(struct repeating_timer *t){
    // printf("Chamei o timer\n");
    
    DPSigma *dados= (DPSigma*)t->user_data;//linka o user data com a struct de dados que tem o sigma
    // dados->sigma (aqui está o desvio)

    //checagem se o sigma passou dos limites inferiores ou superiores
    if(dados->sigma <=0){
        dados->sigma=0;
    }
    if(dados->sigma >=100){
        dados->sigma=100;
    }

    if(!view_hist){
        oled_clear();
        draw_board();

        draw_bias_line(dados->sigma);

        oled_print_info(dados->sigma);
        if(amostras<=500){
        balls_on_board++;
        amostras++;
        }
        // sigma++;
        // if(sigma>100){
        //     sigma=100;
        // }
        
        for(int8_t i=balls_on_board; i>=0;i--){//for que desenha as esferas caindo
            // printf("Estou imprimindo a bola %d, na direcao %d\n",i,balls[i].direction);
            draw_ball(i);
            if(balls[i].direction){//se dir=true, vai para a direita
                balls[i].oled_addr[0]+=STEP/2;
                balls[i].real_addr[0]+=1;
            } else{//se não vai para a esquerda
                balls[i].oled_addr[0]-=STEP/2;
                balls[i].real_addr[0]-=1;
            }

            balls[i].oled_addr[1]+=STEP/2;
            balls[i].real_addr[1]++;
            balls[i].direction=letf_or_right(dados->sigma);
            if(balls[i].real_addr[1]>12){
                // printf("OLED(%d,%d) e Real(%d,%d) Index(%d)\n",balls[i].oled_addr[0],balls[i].oled_addr[1],balls[i].real_addr[0],balls[i].real_addr[1],check_equivalent_hist(balls[i].oled_addr[0]));
                uptade_hist_array(check_equivalent_hist(balls[i].oled_addr[0]));
                deslocar_balls_array(dados->sigma);
            }
        }
        render_on_display(ssd, &frame_area);
    }else{
        
        oled_clear();
        draw_hist();
        render_on_display(ssd, &frame_area);
    }
    return true;
  }

void deslocar_balls_array(int8_t sig){
    // Desloca todos os elementos para a esquerda (índice menor)
    for(uint8_t i = 0; i < 49; i++) { // 49 = 50-1 (último índice é 49)
        balls[i] = balls[i + 1]; // Copia próxima posição para a atual
    }
    // Reinicializa a última posição com valores padrão
    balls[49].direction = letf_or_right(sig);   // Função hipotética que determina direção
    balls[49].oled_addr[0] = 64;              // X inicial
    balls[49].oled_addr[1] = 6;               // Y inicial
    balls[49].real_addr[0] = 6;               // Posição lógica X
    balls[49].real_addr[1] = 1;               // Posição lógica Y
}
void uptade_hist_array(uint8_t b_index){
    hist_array[b_index]++;
    balls_on_board--;
}

void draw_hist(){
    uint16_t max={0};
    for(uint8_t i=0; i<13;i++) {//for para calcular total do array do histograma e pegar o valor máximo
        if(max<hist_array[i]){
            max=hist_array[i];
        }
        // printf("hist_array[%d] = %d\n",i,hist_array[i]);
        // printf("Max = %d \n", max);
    }
    float proportion=(float)63/max;
    printf("proporcao = %f | max = %d \n", proportion, max);
    for(uint8_t i=0; i<13; i++){//for para desenhar histograma

        uint8_t y_addr=proportion*hist_array[i];
        uint8_t x_addr=ESPACE_HIST+i*9;
        // printf("x_addr = %d | y_addr = %d \n", x_addr, y_addr);
        ssd1306_fill_rect(ssd,x_addr,63-y_addr,x_addr+9,63,true);
    }
}

void toggle_view_hist(){
    view_hist = !view_hist;
}
void add_ball_on_board(){
    balls_on_board++;
    amostras++;
}

uint8_t check_equivalent_hist(uint8_t addr) {
    switch (addr) {
        case 16: return 0;
        case 24: return 1;
        case 32: return 2;
        case 40: return 3;
        case 48: return 4;
        case 56: return 5;
        case 64: return 6;
        case 72: return 7;
        case 80: return 8;
        case 88: return 9;
        case 96: return 10;
        case 104: return 11;
        case 112: return 12;
        default:  return 0xFF; // Valor padrão explícito para entradas inválidas
    }
}
module spi_master (
    input wire      clk,
    input wire      rst_n,
    
    input wire [31:0] write_data,
    input wire        write_en,
    input wire        read_en,
    input wire [3:0]  addr,
    output reg [31:0] read_data,
    
    input wire     miso,
    output reg     sclk,
    output reg     mosi,
    output reg     cs_n
 );

 wire [1:0] sel = addr[3:2];
 reg en, busy, done, half;
 reg [7:0] clkdiv, tx, rx, cnt, tx_shift, rx_shift;
 reg [2:0] bits;
 localparam IDLE=0, XFER=1, DONE_ST =2;
 reg [1:0] state;
 
 wire start = write_en && sel == 0 && write_data[1] && (en || write_data[0]) && state ==IDLE && !busy;
 
 always @(posedge clk or negedge rst_n) begin
     if (!rst_n) begin
         {en,busy,done,half} <= 0; state <= IDLE;
         {clkdiv,tx,rx,cnt,tx_shift,rx_shift,bits} <= 0;
         sclk <= 0; mosi <=0; cs_n <= 1;
     end else begin
         if (write_en) begin
           case (sel)
            0: {clkdiv,en} <= {write_data[15:8], write_data[0]};
            1: tx <= write_data[7:0];
            3: if (write_data[1]) done <= 0;
           endcase
         end
         
         case (state)
             IDLE: begin
                sclk<=0; cs_n<=1; busy<=0; cnt<=0; bits<=0; half<=0;
                if (start) begin
                   tx_shift<=tx; mosi<=tx[7]; rx_shift<=0;
                   cs_n<=0; busy<=1; state<=XFER;
                end
             end
             XFER: begin
                 if (cnt==clkdiv) begin
                    cnt<=0;
                    if (!half) begin
                       sclk <=1; rx_shift<={rx_shift[6:0],miso}; half<=1;
                    end else begin
                        sclk<=0; tx_shift<={tx_shift[6:0],1'b0}; mosi<=tx_shift[6];
                        half<=0;
                        if (bits==7) state<=DONE_ST; else bits<=bits+1;
                    end
                 end else cnt<=cnt+1;
             end
             DONE_ST: begin
                 cs_n<=1; sclk<=0; mosi<=0; busy<=0; done<=1;
                 rx<=rx_shift;  state<=IDLE;
             end
        endcase
     end
  end
  
  always @(*) case (sel)       
      0: read_data = {16'h0, clkdiv, 6'b0, 1'b0,en};
      2: read_data = {24'h0, rx};
      3: read_data = {29'b0, 1'b0, done, busy};
      default: read_data = 0;
  endcase
endmodule
                                           
   

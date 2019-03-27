#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    this->init_std_values();


    
    //fd = open(device, O_RDWR);
    this->on_actionOpen_device_triggered();

    spiiotr.tx_buf = (unsigned long)tx;
    spiiotr.rx_buf = (unsigned long)rx;
    spiiotr.len = len;
    spiiotr.delay_usecs = delay;
    spiiotr.speed_hz = speed;
    spiiotr.bits_per_word = bits;

    this->check_spi();

    ui->qplot->xAxis->setRange(0,samples);
    ui->qplot->yAxis->setRange(0,4096);
//    this->on_actionRefresh_triggered();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init_std_values()
{
    samples = 16384*2;
    device = "/dev/spidev0.0";
    mode = 0;
    bits = 8;
    speed =  5000000;
    delay = 0;
    transfer_size = 2;

    //struct timeval start, stop;
    len = transfer_size;
    values = new uint16_t[samples];
    fftvalues = new uint16_t[samples];
    tx = new uint8_t[len];
    rx = new uint8_t[len];
}

void MainWindow::check_spi()
{

    int ret;
    ret = ioctl(fd, SPI_IOC_WR_MODE32, &mode);
    if (ret == -1) std::cerr << "write mode error \n";
    ret = ioctl(fd, SPI_IOC_RD_MODE32, &mode);
    if (ret == -1) std::cerr << "read mode error \n";
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1) std::cerr << "write bits error \n";
    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1) std::cerr << "read bits error \n";
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1) std::cerr << "write speed error \n";
    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1) std::cerr << "read speed error \n";
    printf("spi mode: 0x%x\n", mode);
    printf("bits per word: %d\n", bits);
    printf("max speed: %d Hz (%d KHz)\n\n", speed, speed/1000);
    printf("  speed: %i\n",speed);
    printf("samples: %i\n",samples);


}

void MainWindow::on_actionOpen_device_triggered()
{
    //fd = open(device, O_RDWR);
    fd = open("/dev/spidev0.0", O_RDWR);
}

void MainWindow::on_actionRefresh_triggered()
{
    QVector<double> xh;
    QVector<double> xhfft;
    QVector<double> yh;
    QVector<double> yhfft;
    
    double max_x = 0;
    double max_y = -999999999999.9;
    double min_y = 999999999999;
    int sampesToIgnoreAtBegin = 4;
    xh.resize(samples);
    yh.resize(samples);
    xhfft.resize(samples/2);
    yhfft.resize(samples/2);

    int ret;
    unsigned char *data;

    /*
    struct spi_ioc_transfer spi;
    memset(&spi, 0, sizeof (spi));
    spi.tx_buf        = (unsigned long)(data); // transmit from "data"
    spi.rx_buf        = (unsigned long)(data) ; // receive into "data"
    spi.len           = samples ;
    spi.delay_usecs   = 0 ;
    spi.speed_hz      = speed ;
    spi.bits_per_word = bits ;
    spi.cs_change = 0;

    ret = ioctl(fd, SPI_IOC_MESSAGE(samples), &spi) ;

    for (int i = 0 ; i < samples ; i=i+2)
    {
        values[i] =  ( ( data[i] & 0x1f) << 7 ) + ( ( data[i+1] & 0xfe) >> 1) ;
        std::cout << i << ": " << std::bitset<8>(data[i]) << ":" << std::bitset<8>(data[i+1]) << ":" << values[i] << "\n";
    }*/

    int nfft = samples;

    kiss_fft_cpx  *cin = new kiss_fft_cpx[nfft];
    kiss_fft_cpx *cout = new kiss_fft_cpx[nfft];

    for  ( int i = 0; i < samples;i++) {
        //ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
        //if (ret == -1) std::cerr << "write speed error \n";
        memset(&spiiotr, 0, sizeof (spiiotr));
        spiiotr.tx_buf = (unsigned long)tx;
        spiiotr.rx_buf = (unsigned long)rx;
        spiiotr.len = len;
        spiiotr.delay_usecs = delay;
        spiiotr.speed_hz = speed;
        spiiotr.bits_per_word = bits;
        ioctl(fd, SPI_IOC_MESSAGE(1), &spiiotr);
        if (ret == -1) std::cerr << "error \n";
        //std::cout << std::bitset<32>(spiiotr.rx_buf) << "\n";
        //values[i] =  ( ( spiiotr.rx_buf[0] & 0x1f) << 7 ) + ( ( spiiotr.rx_buf[1] & 0xfe) >> 1) ;
        values[i] =  ( ( rx[0] & 0x1f) << 7 ) + ( ( rx[1] & 0xfe) >> 1) ;
        xh[i] = i;
        yh[i] = (double) values[i]*5 / (4096);
        //yh[i] = values[i];
        cin[i].r = values[i];
        cin[i].i = 0;
        //ui->qplot->replot();
        //std::cout << std::bitset<8>(rx[0]) << std::bitset<8>(rx[1]) << ":" << values[i] << "\n";
        }
    ui->qplot->clearGraphs();
    ui->qplot->xAxis->setRange(0,samples);
    ui->qplot->yAxis->setRange(-0.1,5.1);
    //ui->qplot->yAxis->setRange(-4,4100);
    ui->qplot->addGraph();
    ui->qplot->graph(0)->setData(xh,yh);
    ui->qplot->xAxis->setLabel("time [a.u.]");
    ui->qplot->yAxis->setLabel("voltage [V]");
    ui->qplot->replot();

    kiss_fft_cfg cfg = kiss_fft_alloc( nfft , 0, 0 , 0 );
    kiss_fft(cfg,cin,cout);
    for  ( int i = 0; i < samples/2;i++) {
        xhfft[i] = i;
        yhfft[i] = sqrt( (double) cout[i].r * cout[i].r + (double) cout[i].i * cout[i].i );
        if ((yhfft[i] > max_y) & (i > sampesToIgnoreAtBegin) ) {
            max_x = xhfft[i];
            max_y = yhfft[i];
        }
        if (yhfft[i] < min_y) min_y = yhfft[i];
    }
    std::cout << "Max at: " << max_x << std::endl;
    ui->qplotfft->clearGraphs();
    ui->qplotfft->xAxis->setRange(0.5,samples/2);
//     ui->qplotfft->yAxis->setRange(0.1,10000000);
    ui->qplotfft->yAxis->setRange(min_y,max_y);
    ui->qplotfft->xAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->qplotfft->yAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->qplotfft->addGraph();
    ui->qplotfft->graph(0)->setData(xhfft,yhfft);
    ui->qplotfft->xAxis->setLabel("freq. [a.u.]");
    ui->qplotfft->yAxis->setLabel("dB");
    ui->qplotfft->replot();
}

void MainWindow::loop()
{
    while (osziMode) {    
        this->on_actionRefresh_triggered();
    }
}

void MainWindow::on_actionOszi_mode_triggered(bool checked)
{
    osziMode = checked;
    std::thread t (&MainWindow::loop,this);
    t.detach();

    
//     if (checked) {
//         connect(&daqTimer, SIGNAL(timeout()), this, SLOT(on_actionRefresh_triggered()) );
//         daqTimer.start(0);
//     } else {
//         daqTimer.stop();
//     }
}

void MainWindow::on_actionExit_triggered()
{
    this->~MainWindow();
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#define BUFFER_SIZE 1<<16
#define ARR_SIZE 1<<16
int komutSayisi=0; //history deki komut sayısını tutuyorum

void gecmistenKomutCalistir(char dizi[][100], char komut[100],char buffer[BUFFER_SIZE]) // historyden !"komutno" ile komut çalıştırır
{
    int i = 0, komutNo = 0;
    char s1[100] = "";
    for(i = 0; (s1[i] = komut[i+1]) != '\0'; i++) // !"komutno" dan ! işaretini sökmek için dön
    {
        ;
    }
    komutNo = atoi(s1);
    strcpy(buffer, dizi[komutNo-1]); // historydeki n. komutu buffer a atar ki parse edilebilsin n. komut
}

void gecmisiYazdir(char dizi[][100], int listelenecekKomutSayisi, int toplamKomutSayisi) // history deki istenen komut sayısını yazdırır
{
    int a = 0;
    printf("Sondan başa doğru komutlar: \n");
    for(a = 0; a < listelenecekKomutSayisi; a++) // sondan başa doğru istenilen kadar dön
    {
        printf("%d. komut: %s\n", a+1, dizi[a]);
    }
    printf("Gosterilmesi istenen komut sayisi: %d\n", listelenecekKomutSayisi);
    printf("History'deki toplam komut sayisi: %d\n", toplamKomutSayisi);
}

void gecmisKomutlaraEkle(char dizi[][100], int toplamKomutSayisi, char suankiKomut[100]) // history e yeni kayıt ekler.
{
    int i = toplamKomutSayisi;
    for(i = toplamKomutSayisi; i>0; i--)
    {
        if(i != 10) // 10 elemanlıysa 10.yu 11 e atmaya çalışma
        {
            strcpy(dizi[i],dizi[i-1]);
        }
    }
    strcpy(dizi[0],suankiKomut); // şimdiki komutu en son çalışan komuta ata
}

void sigintYakala(int signum) // ctrl-c yakala
{
    printf("\n%d nolu signal yakalandı.Program sonlanacak...\n", signum);
    exit(EXIT_SUCCESS);
}

void parse_args(char *buffer, char** args,size_t args_size, size_t *nargs)// parse kodu buffer ı parçalar.
{
    char *buf_args[args_size];
    char **cp;
    char *wbuf;
    size_t i,j;
    wbuf=buffer;
    buf_args[0]=buffer;
    args[0]=buffer;

    for(cp=buf_args; (*cp=strsep(&wbuf, " \n\t")) != NULL ;)
    {
        if((*cp != '\0') && (++cp>= & buf_args[args_size]))
            break;
    }

    for (j = i = 0; buf_args[i] != NULL; i++)
    {
        if(strlen(buf_args[i]) > 0)
            args[j++] = buf_args[i];
    }

    *nargs = j;
    args[j] = NULL;
}

void calistir (char buffer[BUFFER_SIZE], char komutGecmisi[][100]) // bir komutu çalıştırmak için kullanılır.
{
    char suankiKomut[100] = "", dosyaAdi[50]="", veri[1035];
    char *args[ARR_SIZE];
    pid_t pid;
    int status, veriYonlendirme = 0, historyParameter=0;
    int z = 0;
    size_t nargs;
    FILE *dosyam;

    parse_args(buffer, args, ARR_SIZE, &nargs); // parse işlemi yapıyorum

    strcpy(suankiKomut, "");
    strcpy(dosyaAdi, "");
    if(args[0] != NULL)
    {
        if(strcmp(args[0], "exit") == 0 && nargs == 1) // komut exit ise kapatıcam programı
            exit(EXIT_SUCCESS);
        else
        {
            if(strcmp(args[0], "exit") == 0 && nargs != 1) // exit parametre almamalı
            {
                printf("exit komutu parametre almaz.\n");
            }
        }
        if(nargs == 1 && args[0][0] == '!') // !komutno gibi bir komutum varsa historyden n. komutu çalıştırıcam
        {
            gecmistenKomutCalistir(komutGecmisi, args[0], buffer); // geçmişten n. komutu buffer a atıyorum.
            calistir(buffer, komutGecmisi); // yeni bufferı alıp geçmişteki komutu çalıştırıyorum.
        }
        else
        {

            if(nargs == 1 && (strcmp(args[0], "history") != 0)) //argüman sayısı 1 olan komutuları çalıştırıyorum
            {
                pid = fork(); // cocuk süreç oluşturuyorum.
                if(pid == 0)
                {
                    strcpy(suankiKomut, args[0]);
                    execvp(suankiKomut, args); // çocuk süreçler komutu çalıştırdım
                    exit(0);
                }
                else
                {
                    wait(&status); // anne çocugu bekliyor.
                }
                if(komutSayisi < 10)
                {
                    komutSayisi++;
                }
                strcpy(suankiKomut, args[0]);
                gecmisKomutlaraEkle(komutGecmisi, komutSayisi, suankiKomut); // çalışan komutu histoye ekler
            }
            if(nargs != 1 && (strcmp(args[0], "history") != 0)) // 1'den fazla argümanı olan komutları çalıştırıyorum.
            {
                if(komutSayisi < 10) //en fazla on komut tutulacak kontrolu
                {
                    komutSayisi++;
                }
                for(z = 0; z < nargs; z++) // çalışak komutu çekiyorum
                {
                    if(z != 0)
                    {
                        strcat(suankiKomut, " ");
                    }
                    strcat(suankiKomut, args[z]);
                }
                gecmisKomutlaraEkle(komutGecmisi, komutSayisi, suankiKomut); //histoye ekliyorum çalışan komutu

                veriYonlendirme=0; // yönlendirme yokmuş gibi varsayıyorum
                for(z = 0; z < nargs; z++) // yönlendirme var mı diye bakıyorum
                {
                    if(strcmp(args[z], "<") == 0)
                        veriYonlendirme = 1; // girdi yönlendirmesi var
                    if(strcmp(args[z], ">") == 0)
                        veriYonlendirme = 2; // çıktı yönlendirmesi var
                    if(strcmp(args[z], "|") == 0)
                        veriYonlendirme = 3; // pipe var
                }
                if(veriYonlendirme == 0) // yönledirme yok
                {
                    pid = fork(); // çocuk süreç oluşturuyorum.
                    if(pid == 0)
                    {
                        strcpy(suankiKomut, args[0]);
                        execvp(suankiKomut, args); // 1'den fazla argümanı olan komutu çalıştırıyorum.
                    }
                    else
                    {
                        wait(&status);
                    }
                }
                if(veriYonlendirme == 2) // çıktı yönlendirmesi varsa
                {
                    pid = fork();
                    if(pid == 0)
                    {
                        strcpy(suankiKomut, args[0]);
                        strcpy(dosyaAdi, args[nargs-1]);
                        for(z=nargs-2; z<nargs; z++)
                        {
                            args[z] = NULL;
                        }
                        printf("Komut verisi %s isimli dosyaya yazıldı.\n", dosyaAdi);
                        freopen(dosyaAdi, "w", stdout); //stdout belirlenmiş dosyaya yönlendiriyorum
                        execvp(suankiKomut, args); // komutu çalıştırıyorum
                        fclose(stdout); // yönlendirmeleri kapatıyorum.
                        freopen("/dev/tty", "w", stdout);
                    }
                    else
                    {
                        wait(&status);
                    }
                }
                if(veriYonlendirme==1) // girdi yönlendirmesi varsa
                {
                    pid = fork();
                    if(pid == 0)
                    {
                        if(strcmp("<", args[0]) != 0) // <'dan önce komut var mı kontrolu
                        {
                            if(nargs != 2) // dosya adi var mı kontrolü
                            {
                                strcpy(suankiKomut, args[0]);
                                strcpy(dosyaAdi, args[nargs-1]);
                                for(z=nargs-2; z<nargs; z++)
                                {
                                    args[z] = NULL;
                                }
                                freopen(dosyaAdi, "r", stdin); // komutun girdisini belirtilen dosyadan alıcam diyorum
                                execvp(suankiKomut, args); //komutu çalıştırıyorum
                                freopen("/dev/tty", "r", stdin);
                            }
                            else
                            {
                                printf("işlem(ler) ve '<' komutlarından sonra dosya adi beklenmektedir.\n");
                                exit(EXIT_SUCCESS);
                            }
                        }
                        else
                        {
                            printf("< dan önce işlem komutu gerekmektedir.\n");
                            exit(EXIT_SUCCESS);
                        }
                    }
                    else
                    {
                        wait(&status); // anne süreç bekliyor...
                    }
                }
                if(veriYonlendirme == 3) // pipeline varsa giriyorum
                {
                    dosyam = popen(suankiKomut,"r"); //pipe ımı oluştudum komutu dosyam'a atıyorum.
                    while(fgets(veri, sizeof(veri), dosyam) != NULL)
                    {
                        printf("%s", veri); // dosyamdaki veriyi ekrana yazırıyorum
                    }
                    pclose(dosyam);
                }
            }
            if(strcmp("history", args[0]) == 0) // history mi yazılcak kontrolu
            {
                if(nargs == 1) // history parametre almamışsa giriyorum
                {
                    strcpy(suankiKomut, "history");
                    if(komutSayisi < 10)
                    {
                        komutSayisi++;
                    }
                    gecmisKomutlaraEkle(komutGecmisi, komutSayisi, suankiKomut);
                    gecmisiYazdir(komutGecmisi, 10, komutSayisi);
                }
                else // history parametre almışsa giriyorum.
                 {
                    strcpy(suankiKomut, "history ");
                    strcat(suankiKomut, args[1]);
                    printf("\n %s \n", suankiKomut);
                    if(komutSayisi < 10)
                    {
                        komutSayisi++;
                    }
                    gecmisKomutlaraEkle(komutGecmisi, komutSayisi, suankiKomut); // geçmiş komutlara ekliyorum önce
                    historyParameter = atoi(args[1]);
                    if((historyParameter <= 10 && historyParameter >= 0) && nargs == 2)
                    {
                        gecmisiYazdir(komutGecmisi, historyParameter, komutSayisi); // sonra yazdırıyorum geçmiş komutları
                    }
                    else
                    {
                        if(nargs != 2)
                        {
                            printf("History komutu sadece bir tamsayı parametre alir.\n");
                        }
                        else
                        {
                            printf("History komutunun parametresi 0'dan kücük 10'dan büyük olamaz.\n");
                        }
                    }
                }
            }
        }
    }
}

int main(int argc, char *argv[], char *envp[]) // ana fonksiyonum
{
    char komutGecmisi[10][100]= {"", "", "", "", "", "", "", "", "", ""}; // history i tuttugum yer.
    struct sigaction yeniIslem;
    yeniIslem.sa_handler = sigintYakala;

    char buffer[BUFFER_SIZE];

    sigaction(SIGINT, &yeniIslem, NULL); // program çalışır çalışmaz ctrl-c bekliyorum.

    while(1)
    {
        printf("#prompt>");
        fgets(buffer, BUFFER_SIZE, stdin); // komut bekliyorum.
        calistir(buffer, komutGecmisi); // komut çalıştırma fonksiyonumu çağırıyprum.
    }
    return 0;
}

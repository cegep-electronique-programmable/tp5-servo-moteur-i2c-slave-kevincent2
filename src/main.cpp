#include <mbed.h>

#define ADDRESSE_I2C_PAR_DEFAUT 0x23

#if !DEVICE_I2CSLAVE
#error [NOT_SUPPORTED] I2C Slave is not supported
#endif

static BufferedSerial serial_port(USBTX, USBRX);

// Utiliser la classe I2CSlave pour créer un objet slave.
// Consulter la documentation pour voir les méthodes disponibles.
I2CSlave slave(I2C_SDA, I2C_SCL);

///////////////////////////////////////////
// Créer un objet debug_led à partir de la classe DigitalOut pour vous aider dans le debuggage
///////////////////////////////////////////
DigitalOut led(LED2);

///////////////////////////////////////////
// Créer un objet moteur à partir de la classe PwmOut
///////////////////////////////////////////
PwmOut moteur(D3);


///////////////////////////////////////////
// Créer une variable pour la machine à état qui gére le moteur (OFF ou Activé)
///////////////////////////////////////////
enum{On, Off};

int etat = Off;

int main() {
  serial_port.set_baud(9600);

  char read_buffer[10];
  char write_buffer[10];

  slave.address(ADDRESSE_I2C_PAR_DEFAUT << 1);

  double pourcentage = 0.075; //milieu
  moteur.period(0.02);

  while (1) {
        
        // Attendre une requête du master
        int i = slave.receive();

        // Traiter la requête
        switch (i) {

            // Si le master envoie une requête de lecture
            case I2CSlave::ReadAddressed:
                ///////////////////////////////////////////
                // Retourner l'état du moteur (sa position ou OFF sous forme d'une chaine de caractères)
                ///////////////////////////////////////////
                //write_buffer[0] = etat;

                slave.write(write_buffer, strlen(write_buffer) + 1); // Includes null char
                break;

            // Si le master envoie une requête de lecture qui nous est adressée
            case I2CSlave::WriteAddressed:
                slave.read(read_buffer, 10);
                printf("Read A: %s\n\r", read_buffer);

                int8_t commande_recue = read_buffer[0];

                ///////////////////////////////////////////
                // Modifier l'état du moteur en fonction de la commande reçue
                ///////////////////////////////////////////
                switch (etat){
                    case Off:
                        moteur.suspend();

                        if(commande_recue == 126){
                            moteur.resume();
                            moteur.write(0.075); //retourne au milieu
                            etat = On;
                        }
                        break;

                    case On:
                        if(commande_recue <= 90 && commande_recue >= -90){
                            pourcentage = ((double)commande_recue * (2/45) + 7.5)/100;
                            moteur.write(pourcentage); 
                        }
                        else if(commande_recue == 127){
                            etat = Off;
                        }
                        break;
                }
                break;
        }
        
        // Vider le buffer de lecture
        for (int i = 0; i < sizeof(read_buffer); i++) {
            read_buffer[i] = 0;
        }
        
        // Vider le buffer d'écriture
        for (int i = 0; i < sizeof(write_buffer); i++) {
            write_buffer[i] = 0;
        }
    }
}
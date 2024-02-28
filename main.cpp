#include <iostream>
#include <windows.h>
#include <openssl/md5.h>
#include <imdisk.h>
#include <QtWidgets>

// Variable global para almacenar la contraseña de administrador (hash)
std::string admin_password_hash;

// Función para calcular el hash MD5 de una cadena de texto
std::string calculate_md5_hash(const std::string& input) {
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5_CTX context;
    MD5_Init(&context);
    MD5_Update(&context, (const unsigned char*)input.c_str(), input.length());
    MD5_Final(digest, &context);

    char md5string[33];
    for(int i = 0; i < 16; i++) {
        sprintf(&md5string[i*2], "%02x", (unsigned int)digest[i]);
    }
    return std::string(md5string);
}

// Función para establecer la contraseña de administrador
void set_admin_password() {
    std::string password;
    std::cout << "Ingrese la nueva contraseña de administrador: ";
    std::cin >> password;
    admin_password_hash = calculate_md5_hash(password);
}

// Función para verificar la contraseña de administrador
bool verify_admin_password() {
    std::string input_password;
    std::cout << "Ingrese la contraseña de administrador: ";
    std::cin >> input_password;
    return calculate_md5_hash(input_password) == admin_password_hash;
}

// Definir la contraseña de administrador (debería ser almacenada de forma más segura en un entorno real)
const std::string ADMIN_PASSWORD = "1234";

// Definir el tamaño del disco virtual en bytes (100 MB)
const DWORD DISK_SIZE = 100 * 1024 * 1024;

// Definir el nombre del disco virtual
const char* DISK_NAME = "Z:";

// Definir el nombre del archivo de copia de seguridad
const char* BACKUP_FILE = "backup.img";

// Variable global para indicar si el sistema está congelado o no
bool frozen = true;

// Función para crear el disco virtual y montarlo como una unidad
void create_virtual_disk() {
    IMDISK_CREATE_DATA create_data = {0};
    create_data.DiskSize.QuadPart = DISK_SIZE;
    create_data.FileNameLength = strlen(DISK_NAME);
    create_data.FileName = (LPWSTR)DISK_NAME;
    create_data.Flags = IMDISK_TYPE_VM | IMDISK_OPTION_RO_COMPAT;
    create_data.DeviceNumber = IMDISK_AUTO_DEVICE_NUMBER;

    if (!ImDiskCreateDevice(GetCurrentProcess(), &create_data, NULL)) {
        std::cerr << "Error al crear el disco virtual: " << GetLastError() << "\n";
        exit(1);
    }

    std::cout << "Disco virtual creado con éxito.\n";
}

// Función para copiar el archivo de copia de seguridad al disco virtual
void copy_backup_to_virtual_disk() {
    std::string command = "copy /Y ";
    command += BACKUP_FILE;
    command += " ";
    command += DISK_NAME;
    system(command.c_str());

    std::cout << "Archivo de copia de seguridad copiado al disco virtual.\n";
}

// Función para copiar el disco virtual al archivo de copia de seguridad
void copy_virtual_disk_to_backup() {
    std::string command = "copy /Y ";
    command += DISK_NAME;
    command += " ";
    command += BACKUP_FILE;
    system(command.c_str());

    std::cout << "Disco virtual copiado al archivo de copia de seguridad.\n";
}

// Función para eliminar el disco virtual y desmontarlo de la unidad
void delete_virtual_disk() {
    IMDISK_REMOVE_DATA remove_data = {0};
    remove_data.FileNameLength = strlen(DISK_NAME);
    remove_data.FileName = (LPWSTR)DISK_NAME;

    if (!ImDiskRemoveDevice(GetCurrentProcess(), 0, &remove_data)) {
        std::cerr << "Error al eliminar el disco virtual: " << GetLastError() << "\n";
        exit(1);
    }

    std::cout << "Disco virtual eliminado con éxito.\n";
}

// Función para pedir la contraseña de administrador al usuario
bool ask_admin_password() {
    std::string input;
    std::cout << "Ingrese la contraseña de administrador: ";
    std::cin >> input;

    if (input == ADMIN_PASSWORD) {
        return true;
    } else {
        std::cerr << "Contraseña incorrecta.\n";
        return false;
    }
}

// Función para activar o desactivar la función de Deep Freeze según la opción del usuario
void toggle_freeze() {
    char option;
    std::cout << "Ingrese A para activar o D para desactivar la función de Deep Freeze: ";
    std::cin >> option;
    option = toupper(option);

    if (option == 'A') {
        frozen = true;
        std::cout << "La función de Deep Freeze ha sido activada.\n";
    } else if (option == 'D') {
        frozen = false;
        std::cout << "La función de Deep Freeze ha sido desactivada.\n";
    } else {
        std::cerr << "Opción inválida.\n";
    }
}

// Clase para crear la interfaz de usuario
class FreezeApp : public QApplication {
public:
    FreezeApp(int argc, char** argv) : QApplication(argc, argv) {
        window = new QWidget();
        button = new QPushButton("Activar/Desactivar Deep Freeze");
        label = new QLabel();
        layout = new QVBoxLayout();
        layout->addWidget(button);
        layout->addWidget(label);
        window->setLayout(layout);
        connect(button, SIGNAL(clicked()), this, SLOT(toggle_freeze()));
        window->show();
        update_label();
    }

    ~FreezeApp() {
        delete button;
        delete label;
        delete layout;
        delete window;
    }

private slots:
    void toggle_freeze() {
        if (ask_admin_password()) {
            ::toggle_freeze();
        }
        update_label();
    }

    void update_label() {
        std::string status_text = "Estado del sistema: ";
        if (frozen) {
            status_text += "<font color='green'>Congelado</font>";
        } else {
            status_text += "<font color='red'>Descongelado</font>";
        }
        label->setText(QString::fromStdString(status_text));
    }

private:
    QWidget* window;
    QPushButton* button;
    QLabel* label;
    QVBoxLayout* layout;
};

int main() {
    // Verificar si se ha establecido una contraseña previamente
    if (admin_password_hash.empty()) {
        std::cout << "Bienvenido al programa de configuración inicial.\n";
        set_admin_password();
        std::cout << "Contraseña de administrador establecida con éxito.\n";
    } else {
        // Verificar la contraseña antes de continuar
        if (!verify_admin_password()) {
            std::cerr << "Contraseña incorrecta. El programa se cerrará.\n";
            return 1;
        }
    }

    std::cout << "Bienvenido al programa.\n";
    // Resto del programa...
    return 0;
}
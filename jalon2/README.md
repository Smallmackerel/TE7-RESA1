# Correction simple — Jalon 1

Cette correction propose une possibilité d'implémentation du jalon 1 où le client envoie un `int` contenant la taille, puis exactement ce nombre d'octets. Le serveur renvoie la taille et les octets au même client. Le serveur un tableau statique de `MAX_CLIENTS` de struct `pollfd`. Une liste chaînée séparée conserve pour chaque client son descripteur et son adresse IPv4 et numéro de port.

## Compilation et lancement

```sh
make
./server 8080
./client 127.0.0.1 8080
```

Le client attend une adresse IPv4 numérique, par exemple `127.0.0.1` ou `192.168.1.10`. Le serveur écoute sur toutes les interfaces IPv4. Tapez `/quit` dans le client pour fermer sa connexion dans le client.
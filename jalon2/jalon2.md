# Jalon 2 — Utilisateurs et messagerie

> Prérequis : les exigences du [jalon 1](jalon1.md) restent obligatoires. À la fin du jalon, soumettez une release `jalon2` contenant votre code dans `travail/`.

## Objectif

Transformez le serveur d'écho en intermédiaire de messagerie. Les utilisateurs choisissent un pseudo, consultent les utilisateurs connectés, obtiennent des informations sur un utilisateur, envoient des messages privés ou diffusent un message à tous les autres utilisateurs.

Les commandes à prendre en charge sont `/nick`, `/who`, `/whois <pseudo>`, `/msgall <message>` et `/msg <pseudo> <message>`.

## Protocole de messages

À partir de ce jalon, les clients et le serveur n'échangent plus directement de simples chaînes. Ils utilisent la structure suivante, déclarée dans `msg_struct.h` :

```c
#define NICK_LEN 128
#define INFOS_LEN 128

enum msg_type {
    NICKNAME_NEW,
    NICKNAME_LIST,
    NICKNAME_INFOS,
    ECHO_SEND,
    UNICAST_SEND,
    BROADCAST_SEND,
    MULTICAST_CREATE,
    MULTICAST_LIST,
    MULTICAST_JOIN,
    MULTICAST_SEND,
    MULTICAST_QUIT,
    FILE_REQUEST,
    FILE_ACCEPT,
    FILE_REJECT,
    FILE_SEND,
    FILE_ACK
};

struct message {
    int pld_len;
    char nick_sender[NICK_LEN];
    enum msg_type type;
    char infos[INFOS_LEN];
};

static char *msg_type_str[] = {
    "NICKNAME_NEW", "NICKNAME_LIST", "NICKNAME_INFOS", "ECHO_SEND",
    "UNICAST_SEND", "BROADCAST_SEND", "MULTICAST_CREATE", "MULTICAST_LIST",
    "MULTICAST_JOIN", "MULTICAST_SEND", "MULTICAST_QUIT", "FILE_REQUEST",
    "FILE_ACCEPT", "FILE_REJECT", "FILE_SEND", "FILE_ACK"
};
```

Chaque échange suit l'ordre suivant :

1. envoyer les octets de `struct message` ;
2. si `pld_len` est non nul, envoyer ensuite exactement `pld_len` octets de *payload*.

`pld_len` est la taille du payload, ou `0` en l'absence de données utiles. `nick_sender` contient le pseudo de l'émetteur. `type` identifie l'action demandée. `infos` contient l'information associée au type.

Pour ce jalon, utilisez `NICKNAME_NEW`, `NICKNAME_LIST`, `NICKNAME_INFOS`, `ECHO_SEND`, `UNICAST_SEND` et `BROADCAST_SEND`.

| Type | Contenu de `infos` |
| --- | --- |
| `NICKNAME_NEW` | Nouveau pseudo. `nick_sender` est vide avant l'attribution d'un pseudo, sinon il contient le pseudo actuel. |
| `NICKNAME_LIST` | Chaîne vide. |
| `NICKNAME_INFOS` | Pseudo de l'utilisateur recherché. |
| `ECHO_SEND` | Chaîne vide. |
| `UNICAST_SEND` | Pseudo du destinataire. |
| `BROADCAST_SEND` | Chaîne vide. |

## Exigences

- **Req2.0** — Tout message respecte impérativement le protocole ci-dessus : structure puis payload éventuel de taille `pld_len`.
- **Req2.1** — Après connexion, l'utilisateur s'identifie avec `/nick <pseudo>` (`NICKNAME_NEW`). Gérez les pseudos trop longs et ceux contenant des espaces ou des caractères autres que des lettres ou chiffres.
- **Req2.2** — Refusez un pseudo déjà attribué avec un message d'erreur.
- **Req2.3** — Le serveur gère plusieurs utilisateurs et connexions. Il stocke dans une liste chaînée les pseudos, sockets et structures d'adresse associés.
- **Req2.4** — Un utilisateur peut changer de pseudo en réutilisant `/nick <pseudo>`.
- **Req2.5** — `/who` (`NICKNAME_LIST`) retourne la liste des utilisateurs connectés.
- **Req2.6** — `/whois <pseudo>` (`NICKNAME_INFOS`) retourne la date de connexion, l'adresse IP et le port du client ciblé.
- **Req2.7** — `/msgall <message>` (`BROADCAST_SEND`) envoie le message à tous les autres utilisateurs.
- **Req2.8** — Un message diffusé ne doit pas être retransmis à son expéditeur.
- **Req2.9** — `/msg <pseudo> <message>` (`UNICAST_SEND`) envoie un message privé au pseudo ciblé.
- **Req2.10** — Si le destinataire d'un `UNICAST_SEND` n'existe pas, le serveur renvoie une information pertinente à l'émetteur.
- **Req2.11** — Sans commande en tête de ligne, le serveur applique la fonction d'écho (`ECHO_SEND`) et renvoie le message à l'utilisateur.

## Exemples d'interaction

```text
/nick CoolestUserEver
[Serveur] : Welcome on the chat CoolestUserEver

/who
[Server] : Online users are
           - User1
           - User2
           - CoolestUserEver

/whois User1
[Server] : User1 connected since 2014/09/29@19:23 with IP address 192.168.3.165 and port number 52322

/msg user1 Hello
[user0] : Hello
```

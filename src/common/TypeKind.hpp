#pragma once
#include <string>
enum class TypeKind {
    Inteiro,
    Tronxo,    
    Papo,      
    Resenha,  
    Vazio,    
    Erro,    
    Indefinido
};

inline const char* to_string(TypeKind kind) {
    switch (kind) {
        case TypeKind::Inteiro: return "inteiro";
        case TypeKind::Tronxo: return "tronxo";
        case TypeKind::Papo: return "papo";
        case TypeKind::Resenha: return "resenha";
        case TypeKind::Vazio: return "vazio";
        case TypeKind::Erro: return "erro";
        case TypeKind::Indefinido: return "indefinido";
        default: return "desconhecido";
    }
    return "desconhecido";
}

inline bool is_numeric(TypeKind kind) {
    return kind == TypeKind::Inteiro || kind == TypeKind::Tronxo;
}

inline bool compatible_types(TypeKind destiny, TypeKind origin){
    if (destiny == origin) return true;
    if (destiny == TypeKind::Tronxo && origin == TypeKind::Inteiro) return true; // Promoção válida
    return false;
}
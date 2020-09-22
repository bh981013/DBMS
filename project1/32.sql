select name
from Pokemon
where id not in(select pid from CatchedPokemon)                
order by name;
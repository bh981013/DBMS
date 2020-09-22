select name
from Trainer
where id in (select owner_id
             from CatchedPokemon
             where pid in(select after_id
                          from Evolution
                          where after_id not in (select before_id from Evolution))
             )
order by name;             